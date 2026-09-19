// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DA/Pal/DC1PalRegistryData.h"

#include "Data/DA/Pal/DC1PalData.h"

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

float UDC1PalRegistryData::GetMaxHealthFor(const UDC1PalData& InPalData, int32 InLevel, int32 InTalent) const
{
	const int32 Level = FMath::Max(1, InLevel);

	return StatGrowth.HealthFlatBase
		+ (StatGrowth.HealthPerLevel * Level)
		+ GetGrowthTerm(InPalData.HealthStat, StatGrowth.HealthCoeff, Level, InTalent);
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
