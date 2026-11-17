// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DA/Pal/DC1PalRegistryData.h"

#include "Data/DA/Pal/DC1PalData.h"
#include "Data/Pal/DC1PalTypes.h"

namespace DC1PalExp
{
	static const FDC1PalExpRow* FindRow(const UDataTable* InTable, int32 InLevel)
	{
		if (nullptr == InTable)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Pal] ExpTable is not set. Check DA_PalRegistry."));
			return nullptr;
		}

		return InTable->FindRow<FDC1PalExpRow>(FName(*FString::FromInt(InLevel)), TEXT("PalExp"), false);
	}
}

float FDC1PalPassiveRankWeights::GetWeight(int32 InRank) const
{
	switch (InRank)
	{
	case -3:	return RankMinus3;
	case -1:	return RankMinus1;
	case  1:	return Rank1;
	case  2:	return Rank2;
	case  3:	return Rank3;
	case  4:	return Rank4;
	}

	return 0.f;
}

TSoftObjectPtr<UDC1PalData> UDC1PalRegistryData::FindPalDataByTag(const FGameplayTag& SpeciesTag) const
{
	const TSoftObjectPtr<UDC1PalData>* Found = PalDataEntries.Find(SpeciesTag);
	if (nullptr == Found)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Find PalData from Species Tag [%s]."), *SpeciesTag.ToString());
		return nullptr;
	}

	return *Found;
}

TSoftObjectPtr<UTexture2D> UDC1PalRegistryData::FindElementIcon(EDC1PalElement InElement) const
{
	const TSoftObjectPtr<UTexture2D>* Found = ElementIcons.Find(InElement);
	if (nullptr == Found)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Find ElementIcon from Element [%d]. Check DA_PalRegistry."), static_cast<int32>(InElement));
		return nullptr;
	}

	return *Found;
}

TSoftObjectPtr<UTexture2D> UDC1PalRegistryData::FindGenderIcon(EDC1PalGender InGender) const
{
	if (EDC1PalGender::None == InGender)
	{
		return nullptr;
	}

	const TSoftObjectPtr<UTexture2D>* Found = GenderIcons.Find(InGender);
	if (nullptr == Found)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Find GenderIcon from Gender [%d]. Check DA_PalRegistry."), static_cast<int32>(InGender));
		return nullptr;
	}

	return *Found;
}

TSoftObjectPtr<UTexture2D> UDC1PalRegistryData::FindPassiveRankIcon(int32 InRank) const
{
	const TSoftObjectPtr<UTexture2D>* Found = PassiveRankIcons.Find(InRank);
	if (nullptr == Found)
	{
		return nullptr;
	}

	return *Found;
}

float UDC1PalRegistryData::GetTalentMultiplier(int32 InTalent) const
{
	const float Ratio = static_cast<float>(FMath::Clamp(InTalent, 0, 100)) / 100.f;
	return 1.f + (Ratio * StatGrowth.TalentMaxBonus);
}

float UDC1PalRegistryData::GetGrowthTerm(float InStat, float InCoeff, int32 InLevel, int32 InTalent) const
{
	const int32 Level = FMath::Max(1, InLevel);

	return InStat * InCoeff * Level * GetTalentMultiplier(InTalent);
}

float UDC1PalRegistryData::GetMaxHealthFor(const UDC1PalData& InPalData, int32 InLevel, int32 InTalent, bool InIsAlpha) const
{
	const int32 Level = FMath::Max(1, InLevel);

	const float MaxHealth = StatGrowth.HealthFlatBase
		+ (StatGrowth.HealthPerLevel * Level)
		+ GetGrowthTerm(InPalData.HealthStat, StatGrowth.HealthCoeff, Level, InTalent);

	return (true == InIsAlpha) ? (MaxHealth * InPalData.AlphaHealthRate) : MaxHealth;
}

float UDC1PalRegistryData::GetAttackPowerFor(const UDC1PalData& InPalData, int32 InLevel, int32 InTalent) const
{
	return StatGrowth.AttackFlatBase
		+ GetGrowthTerm(InPalData.AttackStat, StatGrowth.CombatCoeff, InLevel, InTalent);
}

float UDC1PalRegistryData::GetDefensePowerFor(const UDC1PalData& InPalData, int32 InLevel, int32 InTalent) const
{
	return StatGrowth.DefenseFlatBase
		+ GetGrowthTerm(InPalData.DefenseStat, StatGrowth.CombatCoeff, InLevel, InTalent);
}

float UDC1PalRegistryData::GetElementMultiplier(EDC1PalElement InAttackElement, EDC1PalElement InTargetElement, EDC1PalElement InTargetElement2) const
{
	auto EvaluateOne = [this, InAttackElement](EDC1PalElement InDefendElement) -> float
		{
			const FDC1ElementMatchup* DefendRow = ElementChart.Find(InDefendElement);
			if (nullptr == DefendRow)
			{
				return 1.f;
			}

			return (true == DefendRow->WeakTo.Contains(InAttackElement)) ? ElementStrongMultiplier : 1.f;
		};

	float Multiplier = EvaluateOne(InTargetElement);


	if (EDC1PalElement::Neutral != InTargetElement2 && InTargetElement2 != InTargetElement)
	{
		Multiplier *= EvaluateOne(InTargetElement2);
	}

	return Multiplier;
}

const FDC1PalSkillDef* UDC1PalRegistryData::FindSkillDef(const FGameplayTag& InSkillTag) const
{
	return SkillChart.Find(InSkillTag);
}

const FDC1PalPassiveDef* UDC1PalRegistryData::FindPassiveDef(const FGameplayTag& InPassiveTag) const
{
	const FDC1PalPassiveDef* Found = PassiveChart.Find(InPassiveTag);
	if (nullptr == Found)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Find PassiveDef from Tag [%s]. Check DA_PalRegistry."), *InPassiveTag.ToString());
		return nullptr;
	}

	return Found;
}

void UDC1PalRegistryData::RollPassives(TArray<FGameplayTag>& OutPassiveTags) const
{
	OutPassiveTags.Reset();

	if (0 == PassiveCountWeights.Num() || 0 == PassiveChart.Num())
	{
		return;
	}

	// [1] 몇 개 붙을지
	float CountWeightSum = 0.f;
	for (const float CountWeight : PassiveCountWeights)
	{
		CountWeightSum += CountWeight;
	}

	if (CountWeightSum <= 0.f)
	{
		return;
	}

	int32 RollCount = 0;
	{
		float Pick = FMath::FRandRange(0.f, CountWeightSum);
		for (int32 Index = 0; Index < PassiveCountWeights.Num(); ++Index)
		{
			Pick -= PassiveCountWeights[Index];

			// 마지막 칸을 조건에 넣어둔다
			if (Pick <= 0.f || (PassiveCountWeights.Num() - 1) == Index)
			{
				RollCount = Index;
				break;
			}
		}
	}

	RollCount = FMath::Min(RollCount, MaxPassiveSlots);
	if (RollCount <= 0)
	{
		return;
	}

	// [2] 어떤 걸 붙일지
	TArray<FGameplayTag> Candidates;
	PassiveChart.GetKeys(Candidates);

	for (int32 Slot = 0; Slot < RollCount && Candidates.Num() > 0; ++Slot)
	{
		float RankWeightSum = 0.f;
		for (const FGameplayTag& Candidate : Candidates)
		{
			RankWeightSum += PassiveRankWeights.GetWeight(PassiveChart[Candidate].Rank);
		}

		// 남은 후보가 전부 가중치 0 이면 더 뽑을 게 없다
		if (RankWeightSum <= 0.f)
		{
			return;
		}

		float Pick = FMath::FRandRange(0.f, RankWeightSum);
		for (int32 Index = 0; Index < Candidates.Num(); ++Index)
		{
			Pick -= PassiveRankWeights.GetWeight(PassiveChart[Candidates[Index]].Rank);

			if (Pick <= 0.f || (Candidates.Num() - 1) == Index)
			{
				OutPassiveTags.Add(Candidates[Index]);
				Candidates.RemoveAtSwap(Index);
				break;
			}
		}
	}
}

void UDC1PalRegistryData::GetPassiveMultipliers(const TArray<FGameplayTag>& InPassiveTags, float& OutAttack, float& OutDefense, float& OutWorkSpeed) const
{
	OutAttack = 1.f;
	OutDefense = 1.f;
	OutWorkSpeed = 1.f;

	for (const FGameplayTag& PassiveTag : InPassiveTags)
	{
		const FDC1PalPassiveDef* PassiveDef = FindPassiveDef(PassiveTag);
		if (nullptr == PassiveDef)
		{
			continue;
		}

		OutAttack *= PassiveDef->AttackMultiplier;
		OutDefense *= PassiveDef->DefenseMultiplier;
		OutWorkSpeed *= PassiveDef->WorkSpeedMultiplier;
	}
}

int32 UDC1PalRegistryData::GetDropExp(int32 InLevel) const
{
	const FDC1PalExpRow* Row = DC1PalExp::FindRow(ExpTable, InLevel);
	return (nullptr != Row) ? Row->DropExp : 0;
}

int32 UDC1PalRegistryData::GetDropExpFor(const UDC1PalData* InPalData, int32 InLevel, bool InIsAlpha) const
{
	const float ExpRate = (true == InIsAlpha && nullptr != InPalData) ? InPalData->AlphaExpRate : 1.f;
	return FMath::RoundToInt(GetDropExp(InLevel) * ExpRate);
}

int32 UDC1PalRegistryData::GetExpToNextLevel(int32 InLevel) const
{
	if (InLevel >= MaxLevel)
	{
		return 0;
	}

	// PalNextExp 는 다음 행 기준
	const FDC1PalExpRow* Row = DC1PalExp::FindRow(ExpTable, InLevel + 1);
	return (nullptr != Row) ? Row->PalNextExp : 0;
}

int32 UDC1PalRegistryData::ApplyExperience(FDC1PalInstance& InOutPal, int32 InAmount) const
{
	if (InAmount <= 0 || InOutPal.Level >= MaxLevel)
	{
		return 0;
	}

	InOutPal.Experience += static_cast<float>(InAmount);

	int32 LevelsGained = 0;
	while (InOutPal.Level < MaxLevel)
	{
		const int32 Need = GetExpToNextLevel(InOutPal.Level);
		if (Need <= 0 || InOutPal.Experience < static_cast<float>(Need))
		{
			break;
		}

		InOutPal.Experience -= static_cast<float>(Need);
		++InOutPal.Level;
		++LevelsGained;
	}

	// 최대 레벨
	if (InOutPal.Level >= MaxLevel)
	{
		InOutPal.Experience = 0.f;
	}

	// 레벨업 회복 (기절 제외)
	if (0 < LevelsGained && InOutPal.CurrentHealth > 0.f)
	{
		if (const UDC1PalData* PalData = FindPalDataByTag(InOutPal.SpeciesTag).LoadSynchronous())
		{
			InOutPal.CurrentHealth = GetMaxHealthFor(*PalData, InOutPal.Level, InOutPal.TalentHealth, InOutPal.bIsAlpha);
		}
	}

	return LevelsGained;
}
