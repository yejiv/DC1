// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Define/DC1Enums.h"
#include "DC1PalRegistryData.generated.h"

class UDC1PalData;
class UTexture2D;

UCLASS()
class DC1_API UDC1PalRegistryData : public UDataAsset
{
	GENERATED_BODY()

public:
	TSoftObjectPtr<UDC1PalData>	FindPalDataByTag(const FGameplayTag& SpeciesTag) const;

	TSoftObjectPtr<UTexture2D>	FindElementIcon(EDC1PalElement InElement) const;

	TSoftObjectPtr<UTexture2D>	FindGenderIcon(EDC1PalGender InGender) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal")
	TMap<FGameplayTag, TSoftObjectPtr<UDC1PalData>>	PalDataEntries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|UI")
	TMap<EDC1PalElement, TSoftObjectPtr<UTexture2D>>	ElementIcons;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|UI")
	TMap<EDC1PalGender, TSoftObjectPtr<UTexture2D>>	GenderIcons;
};
