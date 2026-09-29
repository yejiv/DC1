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
	
	// 임시. UDC1PlayerSet 에 CapturePower 어트리뷰트가 생기면 던진 사람에게서 읽는다.
	const float CapturePower = 0.f;

	// ① 레벨 차 — 스피어 등급이 팰 레벨과 직접 겨룬다
	const float LevelGap = SphereLevel + (CapturePower * 0.5f) - InPalLevel;

	// ② 0~1 로 정규화
	const float NormalizedLevel = FMath::Clamp((LevelGap + 50.f) / 99.f, 0.f, 1.f);

	// ③ 체력을 곱한다. 만피 0.769 ~ 빈사 1.0
	const float Combined = FMath::Pow(1.3f, -InHealthRatio) * NormalizedLevel;

	// ④ 9제곱 곡선. 0.35~0.55 구간에서 기울기가 급해 체력을 깎는 값어치가 커진다
	const float BaseChance = (Combined >= 0.5f)
		? (1.f - (FMath::Pow(2.f - (2.f * Combined), 9.f) / 2.f))
		: (256.f * FMath::Pow(Combined, 9.f));

	// ⑤ 종별 보정. 상태이상·월드설정 배율은 그 시스템이 없어 넣지 않았다
	return FMath::Clamp(BaseChance * InCaptureRate, 0.f, 1.f);
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
		Destroy();
		return;
	}

	bCaptureStarted = true;
	TargetPal = Pal;

	SetLifeSpan(0.f);	//사라짐 방지

	const float PalLevel = static_cast<float>(Pal->GetLevel());

	const float Chance = ComputeCaptureChance(Pal->GetHealthRatio(), Pal->GetCaptureRate(), PalLevel);

	// 가중치를 비워두면 단계가 없어져 판정이 돌지 않는다. 최소 한 단계는 보장한다
	if (0 == ShakeWeights.Num())
	{
		ShakeWeights.Add(1.f);
	}

	ShakeWeightSum = 0.f;
	for (const float Weight : ShakeWeights)
	{
		ShakeWeightSum += FMath::Max(0.f, Weight);
	}

	if (ShakeWeightSum <= 0.f)
	{
		ShakeWeightSum = static_cast<float>(ShakeWeights.Num());
	}

	CaptureChance = Chance;
	ShakeIndex = 0;

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
	bSphereOpen = false;

	GetWorldTimerManager().SetTimer(CaptureTimer, this, &ThisClass::OnShake, ShakeInterval, true);
}

void ADC1PalSphere::OnShake()
{
	const int32 StageCount = ShakeWeights.Num();

	// 이번 단계를 버틸 확률 = 전체확률 ^ (이 단계 비중 / 비중 합).
	const float StageExponent = FMath::Max(0.f, ShakeWeights[ShakeIndex]) / ShakeWeightSum;
	const float StageChance = FMath::Pow(CaptureChance, StageExponent);

	++ShakeIndex;

	if (FMath::FRand() >= StageChance)
	{
		Resolve(false);
		return;
	}

	if (ShakeIndex >= StageCount)
	{
		Resolve(true);
		return;
	}
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