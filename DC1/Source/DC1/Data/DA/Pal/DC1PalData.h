// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "Define/DC1Enums.h"
#include "DC1PalData.generated.h"

class UAnimMontage;
class UTexture2D;
class ADC1PalProjectile;
class ADC1PalCharacter;

/**
 *
 */
USTRUCT(BlueprintType)
struct FDC1PalMontageSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal")
	TArray<TSoftObjectPtr<UAnimMontage>>	Montages;

	TSoftObjectPtr<UAnimMontage> PickRandomMontage() const
	{
		return (0 < Montages.Num()) ? Montages[FMath::RandRange(0, Montages.Num() - 1)] : nullptr;
	}
};

// 드랍
USTRUCT(BlueprintType)
struct FDC1PalDropDef
{
	GENERATED_BODY()

	// DT_ItemData 의 ItemID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop", meta = (GetOptions = "DC1.DC1ItemOptionsLibrary.GetItemIDOptions"))
	FName							ItemID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "1"))
	int32							MinCount = { 1 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "1"))
	int32							MaxCount = { 1 };

	// 0~1
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float							Chance = { 1.f };
};

UCLASS()
class DC1_API UDC1PalData : public UDataAsset
{
	GENERATED_BODY()

public:
#pragma region Species
	// 이 종을 소환할 때 쓸 폰. 데이터에서 액터로 되돌릴 떄 필요
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Species")
	TSoftClassPtr<ADC1PalCharacter>	PalClass;

	// 속성. 상성 계산과 UI 색/아이콘 구분에 쓴다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Species")
	EDC1PalElement					Element = { EDC1PalElement::Neutral };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Species")
	EDC1PalElement					Element2 = { EDC1PalElement::Neutral };
#pragma endregion

#pragma region UI
	// 파티 HUD 등에서 쓰는 종족 아이콘. 개체가 아니라 종 단위 값이라 여기에 둔다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSoftObjectPtr<UTexture2D>		Icon;

	// 화면에 띄울 종 이름. 비면 Nickname 폴백도 없으니 종마다 채운다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	FText							DisplayName;
#pragma endregion

#pragma region Locomotion
	// 추격 속도. AnimInstance의 NormalizedSpeed 분모
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	float							RunSpeed = { 400.f };

	// 배회 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	float							WalkSpeed = { 150.f };

	// 도망 속도. 플레이어보다 조금 느려야 잡힘
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	float							FleeSpeed = { 500.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	float							RotationRateYaw = { 360.f };
#pragma endregion

#pragma region Ride
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride")
	bool							bRideable = { false };

	// 스켈레톤 소켓
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable"))
	FName							RideSocketName = { TEXT("RideMarker") };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable", ClampMin = "0.0"))
	float							RideSpeed = { 700.f };

	// 올라타기 시작 / 내리기 끝 위치. 팰 방향 (X 앞, Y 오른쪽). XY 는 탑승 소켓 기준, Z 는 팰 발밑 기준
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable"))
	TMap<EDC1RideDirection, FVector>	RideMountOffsets = {
		{ EDC1RideDirection::Front,	FVector(150.f, 0.f, 0.f) },
		{ EDC1RideDirection::Left,	FVector(0.f, -120.f, 0.f) },
		{ EDC1RideDirection::Right,	FVector(0.f, 120.f, 0.f) },
		{ EDC1RideDirection::Rear,	FVector(-150.f, 0.f, 0.f) },
	};

	// 탄 사람이 트는 몽타주. 옆 ↔ 등 이동 구간은 몽타주의 DC1 Ride Move 노티파이로 찍는다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable"))
	TMap<EDC1RideDirection, TSoftObjectPtr<UAnimMontage>>	RideMountMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable"))
	TSoftObjectPtr<UAnimMontage>	RideDismountMontage;

	// 내리는 쪽. 탄 방향과 상관없이 항상 여기로 내린다 (자리는 RideMountOffsets)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable"))
	EDC1RideDirection				RideDismountDirection = { EDC1RideDirection::Left };

	// 탄 사람 몽타주 길이에 맞춘다. 서버는 이 시간 뒤에 탑승·하차를 끝낸다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable", ClampMin = "0.01"))
	float							RideMountDuration = { 1.2f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ride", meta = (EditCondition = "bRideable", ClampMin = "0.01"))
	float							RideDismountDuration = { 1.f };
#pragma endregion

#pragma region Stats
	// 레벨당 성장 계수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "0.0"))
	float							HealthStat = { 100.f };

	// 야생으로 나올 때의 레벨 범위
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1"))
	int32							MinLevel = { 1 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1"))
	int32							MaxLevel = { 1 };
#pragma endregion

#pragma region Work
	// 거점 작업 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Work", meta = (ClampMin = "0.0"))
	float							WorkSpeedStat = { 10.f };

	// 작업 적성. 없는 작업은 넣지 않는다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Work", meta = (Categories = "Work"))
	TMap<FGameplayTag, int32>		WorkSuitability;
#pragma endregion

#pragma region Ability
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta = (Categories = "Asset"))
	FGameplayTag					AbilityDataTag;

	// 종별 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta = (Categories = "Anim.Pal"))
	TMap<FGameplayTag, FDC1PalMontageSet>	Montages;
#pragma endregion

#pragma region Idle
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Idle", meta = (Categories = "Anim.Pal.Idle"))
	TArray<FGameplayTag>			IdleMontageTags;
#pragma endregion

#pragma region Combat
	// AI가 쓸 스킬 후보.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (Categories = "Pal.Skill"))
	TArray<FGameplayTag>			SkillTags;

	// 성장 계수. 최종값은 GetAttackPowerAtLevel() 이 계산
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float							AttackStat = { 100.f };

	// 성장 계수. 최종값은 GetDefensePowerAtLevel() 이 계산
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float							DefenseStat = { 100.f };

	// 이 종이 어떤 탄을 쏘는가. 키는 스킬 태그. 메시/속도/속성은 투사체 BP가 갖는다. 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (Categories = "Ability.Pal"))
	TMap<FGameplayTag, TSubclassOf<ADC1PalProjectile>>	Projectiles;
#pragma endregion

#pragma region Capture
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Capture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float							CaptureRate = { 0.5f };
#pragma endregion

#pragma region Drop
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop")
	TArray<FDC1PalDropDef>			Drops;
#pragma endregion

#pragma region AI
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float							SightRadius = { 1500.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float							LoseSightRadius = { 2000.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float							PeripheralVisionAngleDegrees = { 70.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float							StimulusMaxAge = { 5.f };

	// 추격 유지 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float							TargetMemoryDuration = { 3.f };

	// 시야에서 놓쳐도 따라가는 시간. 보이는 동안은 계속 갱신
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Perception")
	float							ProvokeMemoryDuration = { 10.f };

	// 스폰 지점 기준 배회 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Wander")
	float							WanderRadius = { 800.f };
#pragma endregion

public:
	TSoftObjectPtr<UAnimMontage> FindMontageByTag(const FGameplayTag& MontageTag) const
	{
		const FDC1PalMontageSet* Found = Montages.Find(MontageTag);
		return (nullptr != Found) ? Found->PickRandomMontage() : nullptr;
	}

	TSubclassOf<ADC1PalProjectile> FindProjectileByTag(const FGameplayTag& SkillTag) const
	{
		const TSubclassOf<ADC1PalProjectile>* Found = Projectiles.Find(SkillTag);
		return (nullptr != Found) ? *Found : nullptr;
	}

	int32 GetWorkSuitabilityLevel(const FGameplayTag& InWorkType) const
	{
		if (false == InWorkType.IsValid())
		{
			return 0;
		}

		for (const TPair<FGameplayTag, int32>& Pair : WorkSuitability)
		{
			if (true == InWorkType.MatchesTag(Pair.Key))
			{
				return Pair.Value;
			}
		}

		return 0;
	}

	bool CanDoWork(const FGameplayTag& InWorkType) const
	{
		return GetWorkSuitabilityLevel(InWorkType) > 0;
	}
};
