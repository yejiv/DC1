// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DC1AppearanceRegistryData.generated.h"

class USkeletalMesh;
class UTexture2D;

USTRUCT(BlueprintType)
struct FDC1AppearancePartDef
{
	GENERATED_BODY()

};

UCLASS()
class DC1_API UDC1AppearanceRegistryData : public UDataAsset
{
	GENERATED_BODY()

};
