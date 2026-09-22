// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DC1AppearanceTypes.generated.h"

/**
 *
 *
 */
USTRUCT()
struct FDC1AppearanceData
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag HairTag = { };

	UPROPERTY()
	FGameplayTag HeadTag = { };

	UPROPERTY()
	FLinearColor HairColor = { FLinearColor::White };
	
	UPROPERTY()
	FLinearColor SkinColor = { FLinearColor::White };
};
