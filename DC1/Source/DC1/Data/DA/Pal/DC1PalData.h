// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DC1PalData.generated.h"

class UAnimMontage;

UCLASS()
class DC1_API UDC1PalData : public UDataAsset
{
	GENERATED_BODY()

public:
#pragma region Locomotion
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Locomotion")
	float							RunSpeed = { 400.f };

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

	// 어빌리티 태그 (종별 몽타주)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta = (Categories = "Ability"))
	TMap<FGameplayTag, TSoftObjectPtr<UAnimMontage>>	Montages;
#pragma endregion

#pragma region Combat
	// 몸통 표면 간 거리 기준. (Evaluator의 DistanceToTarget 비슷)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float							AttackRange = { 150.f };

	// 진입보다 크게! 같은 값이면 사거리 경계에서 Chase와 Attack이 매 프레임 바뀜
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float							AttackExitRange = { 220.f };

	// AttackPower 어트리뷰트의 초기값. 런타임 값은 어트리뷰트 쪽이 진실이다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float							AttackPower = { 10.f };

	// 스윙의 굵기. 실제 도달 거리는 AttackRange + 이 반경이다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float							AttackTraceRadius = { 50.f };
#pragma endregion

public:
	TSoftObjectPtr<UAnimMontage> FindMontageByTag(const FGameplayTag& AbilityTag) const
	{
		const TSoftObjectPtr<UAnimMontage>* Found = Montages.Find(AbilityTag);
		return (nullptr != Found) ? *Found : nullptr;
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
#pragma endregion
};
