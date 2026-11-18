// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/Pal/DC1PalSphere.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "UI/Pal/DC1PalCaptureWidget.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Notification/DC1NotificationViewModel.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Define/DC1Helper.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

ADC1PalSphere::ADC1PalSphere()
	: Super()
{
	// 기본 명중 처리 대신 포획 판정으로 교체
	HitCollider->OnComponentHit.RemoveDynamic(this, &ThisClass::OnProjectileHit);
	HitCollider->OnComponentHit.AddDynamic(this, &ThisClass::OnSphereHit);

	// 팀 공용 Projectile 프로필. Pal·지형 Block, 던진 Player Ignore
	HitCollider->SetCollisionProfileName(TEXT("Projectile"));

	// 조준하는 동안 손에 들고 있다가 Launch 로 던진다. 그 전에는 이동·충돌·수명을 멈춰 둔다
	HitCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMovement->bAutoActivate = false;
	InitialLifeSpan = 0.f;

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

	CaptureWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("CaptureWidget"));
	CaptureWidgetComponent->SetupAttachment(HitCollider);
	CaptureWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	CaptureWidgetComponent->SetDrawAtDesiredSize(true);
	CaptureWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CaptureWidgetComponent->SetVisibility(false);
}

void ADC1PalSphere::BeginPlay()
{
	Super::BeginPlay();

	// 데디케이티드 서버에서는 만들어지지 않는다
	CaptureWidgetComponent->InitWidget();
}

void ADC1PalSphere::Launch(const FRotator& InAimRotation)
{
	if (false == HasAuthority() || true == IsLaunched())
	{
		return;
	}

	bLaunched = true;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorRotation(InAimRotation);
	HitCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	ProjectileMovement->Velocity = InAimRotation.Vector() * ProjectileMovement->InitialSpeed;
	ProjectileMovement->Activate(true);

	SetLifeSpan(LaunchLifeSpan);
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

				// 발광 + 트레일을 보여준 뒤 숨김
				CapturePal = Pal;
				PlayAbsorbFX();
				GetWorldTimerManager().SetTimer(HidePalTimer, this, &ThisClass::HideCapturePal, PalGlowDuration, false);
			}

			GetWorldTimerManager().SetTimer(CaptureTimer, this, &ThisClass::BeginShaking, AbsorbDuration, false);
		}
	}

	// 열림 / 닫힘 순간 ( bSphereOpen은 OnRep이 없어서 여기서 감지 )
	if (bPrevSphereOpen != bSphereOpen)
	{
		bPrevSphereOpen = bSphereOpen;
		UGameplayStatics::PlaySoundAtLocation(this, (true == bSphereOpen) ? CaptureOpenSound : CaptureCloseSound, GetActorLocation());

		if (false == bSphereOpen)
		{
			SpawnFX(CloseFX);
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
	DOREPLIFETIME(ADC1PalSphere, CaptureDisplay);
	DOREPLIFETIME(ADC1PalSphere, CapturePal);
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
	// 명중음 - 클라도 자체 시뮬레이션 Hit로 재생
	if (false == bHitSoundPlayed)
	{
		bHitSoundPlayed = true;

		USoundBase* HitSound = (nullptr != Cast<ADC1PalCharacter>(OtherActor)) ? HitPalSound.Get() : DC1SoundHelper::PickRandom(HitGroundSounds);
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, Hit.ImpactPoint);
	}

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

	SetCaptureDisplay(0, CaptureChance, false);

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

	// 남은 단계를 전부 버틸 확률 = 전체확률 ^ (남은 비중 / 비중 합)
	float RemainingWeight = 0.f;
	for (int32 Index = ShakeIndex; Index < StageCount; ++Index)
	{
		RemainingWeight += FMath::Max(0.f, ShakeWeights[Index]);
	}

	SetCaptureDisplay(ShakeIndex, FMath::Pow(CaptureChance, RemainingWeight / ShakeWeightSum), false);
}

void ADC1PalSphere::Resolve(bool bInSuccess)
{
	GetWorldTimerManager().ClearTimer(CaptureTimer);
	GetWorldTimerManager().ClearTimer(HidePalTimer);

	ADC1PalCharacter* Pal = TargetPal.Get();
	TargetPal = nullptr;

	if (false == IsValid(Pal))
	{
		Destroy();
		return;
	}

	if (true == bInSuccess)
	{
		CaptureDisplay.CapturedPal = Pal->Fold();
		OnPalCaptured.Broadcast(CaptureDisplay.CapturedPal);

		SetCaptureDisplay(ShakeIndex, 1.f, true);
		SetLifeSpan(SuccessHoldDuration);
		return;
	}

	Pal->CancelCapture();
	Pal->OnCaptureFailed(GetInstigator());

	Destroy();
}

void ADC1PalSphere::SetCaptureDisplay(int32 InStage, float InChance, bool bInSuccess)
{
	CaptureDisplay.Stage = static_cast<int8>(InStage);
	CaptureDisplay.Chance = InChance;
	CaptureDisplay.bSuccess = bInSuccess;

	ForceNetUpdate();
	RefreshCaptureWidget();
	TryRequestCaptureNotice();
	PlayCaptureStageSound();
}

void ADC1PalSphere::OnRep_CaptureDisplay()
{
	RefreshCaptureWidget();
	TryRequestCaptureNotice();
	PlayCaptureStageSound();
}

void ADC1PalSphere::PlayCaptureStageSound()
{
	// 성공은 단계 번호가 마지막 단계와 같으므로 단계보다 먼저 본다 ( INT8_MAX = 성공음 재생함 )
	if (true == CaptureDisplay.bSuccess)
	{
		if (INT8_MAX != LastSoundStage)
		{
			LastSoundStage = INT8_MAX;
			UGameplayStatics::PlaySoundAtLocation(this, CaptureSuccessSound, GetActorLocation());
			SpawnFX(SuccessFX);
		}
		return;
	}

	// 0단계는 닫힘음과 겹침
	if (CaptureDisplay.Stage <= 0 || CaptureDisplay.Stage == LastSoundStage)
	{
		return;
	}

	LastSoundStage = CaptureDisplay.Stage;
	UGameplayStatics::PlaySoundAtLocation(this, CaptureShakeSound, GetActorLocation());
	SpawnFX(ShakeFX);
}

void ADC1PalSphere::OnRep_CapturePal()
{
	PlayAbsorbFX();
}

void ADC1PalSphere::PlayAbsorbFX()
{
	if (NM_DedicatedServer == GetNetMode() || false == ::IsValid(CapturePal))
	{
		return;
	}

	CapturePal->PlayCaptureGlow(PalGlowMaterial, false, 0.f);

	if (true == ::IsValid(TrailFX))
	{
		UNiagaraComponent* Trail = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, TrailFX, CapturePal->GetActorLocation());
		if (true == ::IsValid(Trail))
		{
			Trail->SetVariablePosition(TEXT("TargetPosition"), GetActorLocation());
		}
	}

	GetWorldTimerManager().SetTimer(AbsorbFXTimer, this, &ThisClass::SpawnAbsorbFX, PalGlowDuration, false);
}

void ADC1PalSphere::SpawnAbsorbFX()
{
	SpawnFX(AbsorbFX);
}

void ADC1PalSphere::HideCapturePal()
{
	if (true == ::IsValid(CapturePal))
	{
		CapturePal->SetActorHiddenInGame(true);
	}
}

void ADC1PalSphere::SpawnFX(UNiagaraSystem* InFX) const
{
	if (NM_DedicatedServer == GetNetMode() || false == ::IsValid(InFX))
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), InFX, GetActorLocation());
}

void ADC1PalSphere::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 판정이 시작된 뒤 성공 없이 사라짐 = 포획 실패
	if (EEndPlayReason::Destroyed == EndPlayReason && INDEX_NONE != CaptureDisplay.Stage && false == CaptureDisplay.bSuccess)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CaptureFailSound, GetActorLocation());
		SpawnFX(FailFX);

		// 다시 보이는 팰이 잠깐 빛났다가 원래 머티리얼로
		if (true == ::IsValid(CapturePal))
		{
			CapturePal->PlayCaptureGlow(PalGlowMaterial, true, FailGlowDuration);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ADC1PalSphere::TryRequestCaptureNotice()
{
	if (true == bCaptureNoticeRequested || false == CaptureDisplay.bSuccess)
	{
		return;
	}

	const APawn* Thrower = GetInstigator();
	const APlayerController* PC = IsValid(Thrower) ? Thrower->GetController<APlayerController>() : nullptr;
	const ULocalPlayer* LocalPlayer = (nullptr != PC && true == PC->IsLocalController()) ? PC->GetLocalPlayer() : nullptr;
	if (nullptr == LocalPlayer)
	{
		return;
	}

	bCaptureNoticeRequested = true;

	if (UDC1ViewModelSubsystem* VMSubsystem = LocalPlayer->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		if (UDC1NotificationViewModel* NotificationViewModel = VMSubsystem->GetViewModel<UDC1NotificationViewModel>())
		{
			NotificationViewModel->PushPalCaptureNotice(CaptureDisplay.CapturedPal);
		}
	}
}

void ADC1PalSphere::RefreshCaptureWidget()
{
	UDC1PalCaptureWidget* CaptureWidget = Cast<UDC1PalCaptureWidget>(CaptureWidgetComponent->GetUserWidgetObject());

	if (nullptr == CaptureWidget)
	{
		return;
	}

	const APawn* Thrower = GetInstigator();
	const bool bShow = (INDEX_NONE != CaptureDisplay.Stage) && IsValid(Thrower) && (true == Thrower->IsLocallyControlled());

	CaptureWidgetComponent->SetVisibility(bShow);

	if (true == bShow)
	{
		CaptureWidget->SetCaptureState(CaptureDisplay.Stage, CaptureDisplay.Chance, CaptureDisplay.bSuccess);
	}
}