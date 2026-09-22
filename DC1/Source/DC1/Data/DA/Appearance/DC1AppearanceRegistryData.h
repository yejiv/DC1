// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Tags/Appearance/DC1GameplayTag_Appearance.h"
#include "DC1AppearanceRegistryData.generated.h"

class USkeletalMesh;
class UTexture2D;

USTRUCT(BlueprintType)
struct FDC1AppearancePartDef
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh = {};

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UTexture2D> Icon = {};
};

UCLASS()
class DC1_API UDC1AppearanceRegistryData : public UDataAsset
{
	GENERATED_BODY()

public:
	const FDC1AppearancePartDef* GetHairData(const FGameplayTag& Tag) const;
	const FDC1AppearancePartDef* GetHeadData(const FGameplayTag& Tag) const;

	const TMap<FGameplayTag, FDC1AppearancePartDef>& GetHairs() const { return Hairs; }
	const TMap<FGameplayTag, FDC1AppearancePartDef>& GetHeads() const { return Heads; }

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material Property Name")
	FName HairColorPropertyName = { "HairColor" };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material Property Name")
	FName SkinColorPropertyName = { "SkinColor" };

private:

	UPROPERTY(EditDefaultsOnly, Category = "Hair")
	TMap<FGameplayTag, FDC1AppearancePartDef> Hairs;

	UPROPERTY(EditDefaultsOnly, Category = "Head")
	TMap<FGameplayTag, FDC1AppearancePartDef> Heads;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Default")
	FGameplayTag DefaultHairTag = { DC1AppearanceTags::Appearance_Hair_01 };

	UPROPERTY(EditDefaultsOnly, Category = "Default")
	FGameplayTag DefaultHeadTag = { DC1AppearanceTags::Appearance_Head_01 };
};
