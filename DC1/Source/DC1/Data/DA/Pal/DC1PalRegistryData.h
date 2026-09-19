// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Define/DC1Enums.h"
#include "DC1PalRegistryData.generated.h"

class UDC1PalData;
class UTexture2D;

// 전 종이 공유하는 스탯 성장 수치. 종별 값은 PalData 의 HealthStat / AttackStat / DefenseStat
//
// 최종 스탯 = 기반값 + (레벨당 고정) + (종 스탯 × 계수 × 레벨 × 개체값배율)

USTRUCT(BlueprintType)
struct FDC1PalStatGrowth
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1.0"))
	float	HealthFlatBase = { 100.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float	HealthPerLevel = { 1.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float	HealthCoeff = { 0.1f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1.0"))
	float	AttackFlatBase = { 100.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1.0"))
	float	DefenseFlatBase = { 50.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float	CombatCoeff = { 0.075f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float	TalentMaxBonus = { 0.3f };
};

USTRUCT(BlueprintType)
struct FDC1ElementMatchup
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<EDC1PalElement>	WeakTo;
};

UCLASS()
class DC1_API UDC1PalRegistryData : public UDataAsset
{
	GENERATED_BODY()

public:
	TSoftObjectPtr<UDC1PalData>	FindPalDataByTag(const FGameplayTag& SpeciesTag) const;

	TSoftObjectPtr<UTexture2D>	FindElementIcon(EDC1PalElement InElement) const;

	TSoftObjectPtr<UTexture2D>	FindGenderIcon(EDC1PalGender InGender) const;

	// 최종 스탯
	float	GetMaxHealthFor(const UDC1PalData& InPalData, int32 InLevel, int32 InTalent) const;
	float	GetAttackPowerFor(const UDC1PalData& InPalData, int32 InLevel, int32 InTalent) const;
	float	GetDefensePowerFor(const UDC1PalData& InPalData, int32 InLevel, int32 InTalent) const;

	// 개체값 배율
	float	GetTalentMultiplier(int32 InTalent) const;

	// 레벨 성장분
	float	GetGrowthTerm(float InStat, float InCoeff, int32 InLevel, int32 InTalent) const;

	// 속성 상성 배율
	float	GetElementMultiplier(EDC1PalElement InAttackElement, EDC1PalElement InTargetElement, EDC1PalElement InTargetElement2) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal")
	TMap<FGameplayTag, TSoftObjectPtr<UDC1PalData>>	PalDataEntries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|UI")
	TMap<EDC1PalElement, TSoftObjectPtr<UTexture2D>>	ElementIcons;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|UI")
	TMap<EDC1PalGender, TSoftObjectPtr<UTexture2D>>	GenderIcons;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Combat")
	FDC1PalStatGrowth								StatGrowth;

	// 속성별 약점
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Combat")
	TMap<EDC1PalElement, FDC1ElementMatchup>		ElementChart;

	// 약점 속성으로 맞았을 때의 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Combat", meta = (ClampMin = "1.0"))
	float											ElementStrongMultiplier = { 2.f };

	// 스킬 속성이 같을 때. "불 팰에게 불 스킬을 주는" 이유를 만든다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Combat", meta = (ClampMin = "1.0"))
	float											ElementMatchBonus = { 1.2f };
};
