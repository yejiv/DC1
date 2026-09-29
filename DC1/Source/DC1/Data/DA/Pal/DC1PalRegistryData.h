// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "Define/DC1Enums.h"
#include "DC1PalRegistryData.generated.h"

class UDC1PalData;
class UTexture2D;
class ADC1PalProjectile;
struct FDC1PalInstance;

// 경험치 표 행. 행 이름 = 레벨
USTRUCT(BlueprintType)
struct FDC1PalExpRow : public FTableRowBase
{
	GENERATED_BODY()

	// 처치·포획 경험치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	int32	DropExp = { 0 };

	// 이전 레벨 → 이 레벨 필요량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	int32	PalNextExp = { 0 };
};

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

USTRUCT(BlueprintType)
struct FDC1PalSkillDef
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FText			DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FText			Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill", meta = (Categories = "Ability.Pal"))
	FGameplayTag	ActivationTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill", meta = (Categories = "Anim.Pal"))
	FGameplayTag	MontageTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	EDC1PalElement	Element = { EDC1PalElement::Neutral };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = "0.0"))
	float			Power = { 4.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = "0.0"))
	float			Cooldown = { 2.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Range", meta = (ClampMin = "0.0"))
	float			MaxRange = { 1200.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Range", meta = (ClampMin = "0.0"))
	float			ApproachRange = { 700.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Range", meta = (ClampMin = "0.0"))
	float			MinRange = { 400.f };

	// 비면 PalData.Projectiles 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TSubclassOf<ADC1PalProjectile>	ProjectileClass;
};

// 패시브
USTRUCT(BlueprintType)
struct FDC1PalPassiveDef
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText	DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText	Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "-3", ClampMax = "4"))
	int32	Rank = { 1 };

	// 어트리뷰트에 곱할 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float	AttackMultiplier = { 1.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float	DefenseMultiplier = { 1.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float	WorkSpeedMultiplier = { 1.f };
};

// 작업 종류. 배열 순서 = UI 표시 순서
USTRUCT(BlueprintType)
struct FDC1PalWorkDef
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Pal.Work"))
	FGameplayTag				WorkTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText						DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D>	Icon;
};

// 랭크별 추첨 가중치
USTRUCT(BlueprintType)
struct FDC1PalPassiveRankWeights
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float	RankMinus3 = { 2.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float	RankMinus1 = { 5.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float	Rank1 = { 8.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float	Rank2 = { 2.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float	Rank3 = { 3.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float	Rank4 = { 1.f };

	float	GetWeight(int32 InRank) const;
};

UCLASS()
class DC1_API UDC1PalRegistryData : public UDataAsset
{
	GENERATED_BODY()

public:
	TSoftObjectPtr<UDC1PalData>	FindPalDataByTag(const FGameplayTag& SpeciesTag) const;

	TSoftObjectPtr<UTexture2D>	FindElementIcon(EDC1PalElement InElement) const;

	TSoftObjectPtr<UTexture2D>	FindGenderIcon(EDC1PalGender InGender) const;

	// 표에 없으면 nullptr - 아이콘을 숨긴다
	TSoftObjectPtr<UTexture2D>	FindPassiveRankIcon(int32 InRank) const;

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

	// 액티브 스킬. 표에 없으면 nullptr - 호출부가 기존 어빌리티 값으로 폴백한다
	const FDC1PalSkillDef*		FindSkillDef(const FGameplayTag& InSkillTag) const;

	// 패시브
	const FDC1PalPassiveDef*	FindPassiveDef(const FGameplayTag& InPassiveTag) const;

	void						RollPassives(TArray<FGameplayTag>& OutPassiveTags) const;

	// 패시브 배율 곱 (GE 의 MultiplyCompound 와 같은 값)
	void						GetPassiveMultipliers(const TArray<FGameplayTag>& InPassiveTags, float& OutAttack, float& OutDefense, float& OutWorkSpeed) const;

	// 경험치
	int32						GetDropExp(int32 InLevel) const;

	// 다음 레벨 필요량. 최대 레벨이면 0
	int32						GetExpToNextLevel(int32 InLevel) const;

	// 경험치 적용 + 레벨업. 오른 레벨 수 반환
	int32						ApplyExperience(FDC1PalInstance& InOutPal, int32 InAmount) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal")
	TMap<FGameplayTag, TSoftObjectPtr<UDC1PalData>>	PalDataEntries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|UI")
	TMap<EDC1PalElement, TSoftObjectPtr<UTexture2D>>	ElementIcons;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|UI")
	TMap<EDC1PalGender, TSoftObjectPtr<UTexture2D>>	GenderIcons;

	// 패시브 Rank(-3 ~ 4) → 화살표 아이콘
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|UI")
	TMap<int32, TSoftObjectPtr<UTexture2D>>			PassiveRankIcons;

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

	// 액티브 스킬 목록. 전 종 공용이다 - 한 줄을 고치면 그 스킬을 쓰는 모든 종이 같이 바뀐다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Skill", meta = (Categories = "Pal.Skill"))
	TMap<FGameplayTag, FDC1PalSkillDef>				SkillChart;

	// 패시브 목록
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Passive")
	TMap<FGameplayTag, FDC1PalPassiveDef>			PassiveChart;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Passive")
	FDC1PalPassiveRankWeights						PassiveRankWeights;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Passive")
	TArray<float>									PassiveCountWeights = { 40.f, 40.f, 20.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Passive", meta = (ClampMin = "1"))
	int32											MaxPassiveSlots = { 4 };

	// 레벨별 경험치 표
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Exp", meta = (RequiredAssetDataTags = "RowStructure=/Script/DC1.DC1PalExpRow"))
	TObjectPtr<UDataTable>							ExpTable = { nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Exp", meta = (ClampMin = "1"))
	int32											MaxLevel = { 80 };

	// 작업 종류 12개. 순서대로 UI 에 나온다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Work")
	TArray<FDC1PalWorkDef>							WorkChart;

	// UI 위력 표기 배율. SkillChart.Power 가 원작 ÷ 5
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Skill", meta = (ClampMin = "0.0"))
	float											SkillPowerDisplayScale = { 5.f };

	// 상자 회복. 초당 최대 체력 비율 (기절 포함)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Box", meta = (ClampMin = "0.0"))
	float											BoxHealRatePerSecond = { 0.01f };
};
