// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DC1PalData.generated.h"

class UAnimMontage;

/**
 * 재생할 때 하나를 랜덤으로 고른다.
 */
USTRUCT(BlueprintType)
struct FDC1PalMontageSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal")
	TArray<TSoftObjectPtr<UAnimMontage>>	Montages;

	TSoftObjectPtr<UAnimMontage> PickRandomMontage() const
	{
		TArray<int32> ValidIndices;
		ValidIndices.Reserve(Montages.Num());

		for (int32 Index = 0; Index < Montages.Num(); ++Index)
		{
			if (false == Montages[Index].IsNull())
			{
				ValidIndices.Add(Index);
			}
		}

		if (0 == ValidIndices.Num())
		{
			return nullptr;
		}

		return Montages[ValidIndices[FMath::RandRange(0, ValidIndices.Num() - 1)]];
	}
};

UCLASS()
class DC1_API UDC1PalData : public UDataAsset
{
	GENERATED_BODY()

public:
#pragma region Locomotion
	// 추격 속도. AnimInstance의 NormalizedSpeed 분모이기도 하다
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

	// 종별 몽타주. 키는 Anim.Pal.* 이다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta = (Categories = "Anim.Pal"))
	TMap<FGameplayTag, FDC1PalMontageSet>	Montages;
#pragma endregion

#pragma region Idle
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Idle", meta = (Categories = "Anim.Pal.Idle"))
	TArray<FGameplayTag>			IdleMontageTags;
#pragma endregion

#pragma region Combat
	// 몸통 표면 간 거리 기준. (Evaluator의 DistanceToTarget 비슷)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float							AttackRange = { 150.f };

	// 진입보다 크게! 같은 값이면 사거리 경계에서 Chase와 Attack이 매 프레임 바뀜
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float							AttackExitRange = { 220.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float							AttackPower = { 10.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float							AttackTraceRadius = { 50.f };
#pragma endregion

public:
	const FDC1PalMontageSet* FindMontageSetByTag(const FGameplayTag& MontageTag) const
	{
		return Montages.Find(MontageTag);
	}

	// 변형이 여러 개면 매 호출마다 다른 것이 나올 수 있다.
	TSoftObjectPtr<UAnimMontage> FindMontageByTag(const FGameplayTag& MontageTag) const
	{
		const FDC1PalMontageSet* Found = FindMontageSetByTag(MontageTag);
		return (nullptr != Found) ? Found->PickRandomMontage() : nullptr;
	}

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

	// 스폰 지점 기준 배회 반경
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Wander")
	float							WanderRadius = { 800.f };
#pragma endregion
};
