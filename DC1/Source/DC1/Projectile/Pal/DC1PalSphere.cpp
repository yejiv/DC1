// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/Pal/DC1PalSphere.h"

#include "Character/Pal/DC1PalCharacter.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/World.h"

ADC1PalSphere::ADC1PalSphere()
	: Super()
{
	// 기본 명중 처리 대신 포획 판정으로 교체
	HitCollider->OnComponentHit.RemoveDynamic(this, &ThisClass::OnProjectileHit);
	HitCollider->OnComponentHit.AddDynamic(this, &ThisClass::OnSphereHit);

	// 팀 공용 Projectile 프로필. Pal·지형 Block, 던진 Player Ignore
	HitCollider->SetCollisionProfileName(TEXT("Projectile"));

	ProjectileMesh->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);

	// 반구 2개 사용
	ProjectileMesh->SetVisibility(false);

	PrimaryActorTick.bCanEverTick = true;	//애니메이션

	TopHalfMesh = CreateDefaultSubobject<UStaticMeshComponent>("TopHalfMesh");
	TopHalfMesh->SetupAttachment(HitCollider);
	TopHalfMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BottomHalfMesh = CreateDefaultSubobject<UStaticMeshComponent>("BottomHalfMesh");
	BottomHalfMesh->SetupAttachment(HitCollider);
	BottomHalfMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//ProjectileMovement->InitialSpeed = 1500.f;
	//ProjectileMovement->MaxSpeed = 1500.f;
	//ProjectileMovement->ProjectileGravityScale = 1.f;
}

void ADC1PalSphere::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 물러남 (서버)
	if ((true == bCaptureStarted) && (RiseAlpha < 1.f))
	{
		RiseAlpha = FMath::Min(1.f, RiseAlpha + (DeltaSeconds / RiseDuration));

		SetActorLocation(FMath::Lerp(RiseStart, RiseEnd, RiseAlpha));

		// 이동 완료
		if (RiseAlpha >= 1.f)
		{
			bSphereOpen = true;

			// 흡수
			ADC1PalCharacter* Pal = TargetPal.Get();

			if (true == IsValid(Pal))
			{
				Pal->BeginCapture();
			}

			GetWorldTimerManager().SetTimer(CaptureTimer, this, &ThisClass::BeginShaking, AbsorbDuration, false);
		}
	}

	const float Target = (true == bSphereOpen) ? 1.f : 0.f;

	OpenAlpha = FMath::FInterpConstantTo(OpenAlpha, Target, DeltaSeconds, OpenSpeed);

	const float Offset = OpenDistance * OpenAlpha;

	TopHalfMesh->SetRelativeLocation(FVector(0.f, 0.f, Offset));
	BottomHalfMesh->SetRelativeLocation(FVector(0.f, 0.f, -Offset));
}

void ADC1PalSphere::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADC1PalSphere, bSphereOpen);
}

float ADC1PalSphere::ComputeCaptureChance(float InHealthRatio, float InCaptureRate, float InPalLevel) const
{
	const float HealthFactor = 1.f - (InHealthRatio * HealthWeight); //HealthWeight 0.8이면 만피 0.2 ~ 빈사 1.0
	const float LevelFactor = 1.f + (FMath::Max(0.f, InPalLevel - 1.f) * LevelPenalty);

	// 임시. 플레이어에 CapturePower 어트리뷰트
	const float CapturePower = 1.f;

	const float Chance = (InCaptureRate * HealthFactor * CapturePower * CaptureMultiplier) / LevelFactor;

	return FMath::Clamp(Chance, 0.f, 1.f);
}

void ADC1PalSphere::OnSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (false == HasAuthority())
	{
		return;
	}

	if (true == bCaptureStarted)
	{
		return;
	}

	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(OtherActor);

	if (false == IsValid(Pal))
	{
		Destroy();
		return;
	}

	// 회수
	if (EDC1Team::Wild != Pal->GetTeam())
	{
		UE_LOG(LogTemp, Log, TEXT("[Pal] Sphere hit %s but it is not wild (Team=%s). Ignored."),
			*GetNameSafe(Pal), *UEnum::GetValueAsString(Pal->GetTeam()));

		Destroy();
		return;
	}

	bCaptureStarted = true;
	TargetPal = Pal;

	SetLifeSpan(0.f);	//사라짐 방지

	// 임시. 팰 레벨 고정
	const float PalLevel = 1.f;

	const float Chance = ComputeCaptureChance(Pal->GetHealthRatio(), Pal->GetCaptureRate(), PalLevel);

	ShakeChance = FMath::Pow(Chance, 1.f / static_cast<float>(ShakeCount));
	ShakeIndex = 0;

	UE_LOG(LogTemp, Log, TEXT("[Pal] Capture: %s  HP %.0f%%  Lv %.0f  chance %.0f%%  (per-stage %.0f%%, x%d)"),
		*GetNameSafe(Pal), Pal->GetHealthRatio() * 100.f, PalLevel,
		Chance * 100.f, ShakeChance * 100.f, ShakeCount);

	HitCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (IsValid(ProjectileMovement))
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	const FRotator ThrowerYaw(0.f, IsValid(GetInstigator()) ? GetInstigator()->GetActorRotation().Yaw : 0.f, 0.f);

	RiseStart = GetActorLocation();
	RiseEnd = RiseStart + ThrowerYaw.RotateVector(RiseOffset);
}

void ADC1PalSphere::BeginShaking()
{
	// 흡수 종료
	bSphereOpen = false;

	GetWorldTimerManager().SetTimer(CaptureTimer, this, &ThisClass::OnShake, ShakeInterval, true);
}

void ADC1PalSphere::OnShake()
{
	++ShakeIndex;

	if (FMath::FRand() >= ShakeChance)
	{
		UE_LOG(LogTemp, Log, TEXT("[Pal] Capture: stage %d/%d broke out.  FAILED"), ShakeIndex, ShakeCount);
		Resolve(false);
		return;
	}

	if (ShakeIndex >= ShakeCount)
	{
		UE_LOG(LogTemp, Log, TEXT("[Pal] Capture: stage %d/%d held.  CAPTURED"), ShakeIndex, ShakeCount);
		Resolve(true);
		return;
	}

	// UI 넘길 값
	const float Remaining = FMath::Pow(ShakeChance, static_cast<float>(ShakeCount - ShakeIndex));

	UE_LOG(LogTemp, Log, TEXT("[Pal] Capture: stage %d/%d held.  remaining %.0f%%"),
		ShakeIndex, ShakeCount, Remaining * 100.f);
}

void ADC1PalSphere::Resolve(bool bInSuccess)
{
	GetWorldTimerManager().ClearTimer(CaptureTimer);

	ADC1PalCharacter* Pal = TargetPal.Get();
	TargetPal = nullptr;

	if (false == IsValid(Pal))
	{
		Destroy();
		return;
	}

	if (true == bInSuccess)
	{
		OnPalCaptured.Broadcast(Pal->Fold());
	}
	else
	{
		Pal->CancelCapture();
		Pal->OnCaptureFailed(GetInstigator());
	}

	Destroy();
}