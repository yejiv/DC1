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
