// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/Pal/DC1PalSphere.h"

#include "Character/Pal/DC1PalCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

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

void ADC1PalSphere::LogPassives(const ADC1PalCharacter& InPal) const
{
	const UAbilitySystemComponent* PalASC = InPal.GetAbilitySystemComponent();
	if (nullptr == PalASC)
	{
		return;
	}

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);

	FString PassiveNames;
	for (const FGameplayTag& PassiveTag : InPal.GetPassiveTags())
	{
		if (false == PassiveNames.IsEmpty())
		{
			PassiveNames += TEXT(", ");
		}

		// 에디터에 넣은 표시 이름을 쓴다. 비어 있으면 태그로 대신해서 입력 누락도 같이 드러난다
		const FDC1PalPassiveDef* PassiveDef = (nullptr != Registry) ? Registry->FindPassiveDef(PassiveTag) : nullptr;

		PassiveNames += (nullptr != PassiveDef && false == PassiveDef->DisplayName.IsEmpty())
			? PassiveDef->DisplayName.ToString()
			: PassiveTag.GetTagName().ToString();
	}

	if (true == PassiveNames.IsEmpty())
	{
		PassiveNames = TEXT("none");
	}

	// BaseValue 는 성장 공식이 낸 값, CurrentValue 는 패시브 배율까지 얹힌 값.
	// 둘을 나란히 찍어야 배율이 실제로 걸렸는지 보인다
	auto ReadPair = [PalASC](const FGameplayAttribute& InAttribute, float& OutBase, float& OutCurrent)
		{
			OutBase = PalASC->GetNumericAttributeBase(InAttribute);
			OutCurrent = PalASC->GetNumericAttribute(InAttribute);
		};

	float BaseAttack = 0.f;
	float CurrentAttack = 0.f;
	float BaseDefense = 0.f;
	float CurrentDefense = 0.f;
	float BaseWork = 0.f;
	float CurrentWork = 0.f;

	ReadPair(UDC1AttributeSet::GetAttackPowerAttribute(), BaseAttack, CurrentAttack);
	ReadPair(UDC1AttributeSet::GetDefensePowerAttribute(), BaseDefense, CurrentDefense);
	ReadPair(UDC1AttributeSet::GetWorkSpeedAttribute(), BaseWork, CurrentWork);

	UE_LOG(LogTemp, Log, TEXT("[Pal] Capture: passives [%s]  Atk %.1f -> %.1f   Def %.1f -> %.1f   Work %.1f -> %.1f"),
		*PassiveNames,
		BaseAttack, CurrentAttack,
		BaseDefense, CurrentDefense,
		BaseWork, CurrentWork);
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

	UE_LOG(LogTemp, Log, TEXT("[Pal] Capture: %s  HP %.0f%%  Lv %.0f  IV(H%d/A%d)  chance %.0f%%  (x%d stages)"),
		*GetNameSafe(Pal), Pal->GetHealthRatio() * 100.f, PalLevel,
		Pal->GetTalentHealth(), Pal->GetTalentAttack(),
		Chance * 100.f, ShakeWeights.Num());

	LogPassives(*Pal);

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
	const int32 StageCount = ShakeWeights.Num();

	// 이번 단계를 버틸 확률 = 전체확률 ^ (이 단계 비중 / 비중 합).
	const float StageExponent = FMath::Max(0.f, ShakeWeights[ShakeIndex]) / ShakeWeightSum;
	const float StageChance = FMath::Pow(CaptureChance, StageExponent);

	++ShakeIndex;

	if (FMath::FRand() >= StageChance)
	{
		UE_LOG(LogTemp, Log, TEXT("[Pal] Capture: stage %d/%d broke out (stage %.0f%%).  FAILED"),
			ShakeIndex, StageCount, StageChance * 100.f);

		Resolve(false);
		return;
	}

	if (ShakeIndex >= StageCount)
	{
		UE_LOG(LogTemp, Log, TEXT("[Pal] Capture: stage %d/%d held (stage %.0f%%).  CAPTURED"),
			ShakeIndex, StageCount, StageChance * 100.f);

		Resolve(true);
		return;
	}

	// 중간 단계를 버텄다. 성공·실패가 갈릴 때만 로그를 남기므로 여기서는 찍지 않는다.
	//
	// UI 가 "여기서부터 잡힐 확률" 을 보여주려면 남은 단계들의 비중을 합쳐
	//   FMath::Pow(CaptureChance, 남은비중합 / ShakeWeightSum)
	// 으로 구한다. 지금은 보여줄 곳이 없어 계산하지 않는다
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