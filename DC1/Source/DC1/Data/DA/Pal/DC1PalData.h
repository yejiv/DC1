// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DC1PalData.generated.h"

class UAnimMontage;

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

UCLASS()
class DC1_API UDC1PalData : public UDataAsset
{
	GENERATED_BODY()

public:
#pragma region Locomotion
	// 추격 속도. AnimInstance의 NormalizedSpeed 분모
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	float							RunSpeed = { 400.f };

	// 배회 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	float							WalkSpeed = { 150.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	float							RotationRateYaw = { 360.f };
#pragma endregion

#pragma region Stats
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float							MaxHealth = { 100.f };
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
	// AI가 쓸 스킬 목록. 앞에 있을수록 우선. 사거리는 어빌리티 BP가 갖는다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (Categories = "Ability.Pal"))
	TArray<FGameplayTag>			SkillTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float							AttackPower = { 10.f };
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
};
