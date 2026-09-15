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
