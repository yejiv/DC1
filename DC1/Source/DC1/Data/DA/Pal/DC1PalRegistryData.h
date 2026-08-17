// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DC1PalRegistryData.generated.h"

class UDC1PalData;

UCLASS()
class DC1_API UDC1PalRegistryData : public UDataAsset
{
	GENERATED_BODY()

public:
	TSoftObjectPtr<UDC1PalData>	FindPalDataByTag(const FGameplayTag& SpeciesTag) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal")
	TMap<FGameplayTag, TSoftObjectPtr<UDC1PalData>>	PalDataEntries;
};
