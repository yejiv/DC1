// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Tags/Appearance/DC1GameplayTag_Appearance.h"
#include "DC1AppearanceRegistryData.generated.h"

class USkeletalMesh;
class UTexture2D;
class UMaterialInterface;

USTRUCT(BlueprintType)
struct FDC1AppearancePartDef
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh = {};

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UTexture2D> Icon = {};
};

// 눈은 메쉬가 아니라 Head 메쉬의 눈 전용 머티리얼 슬롯을 교체하는 방식이라 별도 Def를 둔다
USTRUCT(BlueprintType)
struct FDC1AppearanceMaterialDef
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UMaterialInterface> Material = {};

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
	const FDC1AppearanceMaterialDef* GetEyeData(const FGameplayTag& Tag) const;

	const TMap<FGameplayTag, FDC1AppearancePartDef>& GetHairs() const { return Hairs; }
	const TMap<FGameplayTag, FDC1AppearancePartDef>& GetHeads() const { return Heads; }
	const TMap<FGameplayTag, FDC1AppearanceMaterialDef>& GetEyes() const { return Eyes; }

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material Property Name")
	FName HairColorPropertyName = { "HairColor" };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material Property Name")
	FName SkinColorPropertyName = { "SkinColor" };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material Property Name")
	FName EyeColorPropertyName = { "EyeColor" };

	// Head 스켈레탈 메쉬에서 눈 머티리얼을 교체할 슬롯 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Material Property Name")
	FName EyeMaterialSlotName = { "Eye" };

private:

	UPROPERTY(EditDefaultsOnly, Category = "Hair")
	TMap<FGameplayTag, FDC1AppearancePartDef> Hairs;

	UPROPERTY(EditDefaultsOnly, Category = "Head")
	TMap<FGameplayTag, FDC1AppearancePartDef> Heads;

	UPROPERTY(EditDefaultsOnly, Category = "Eye")
	TMap<FGameplayTag, FDC1AppearanceMaterialDef> Eyes;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Default")
	FGameplayTag DefaultHairTag = { DC1AppearanceTags::Appearance_Hair_01 };

	UPROPERTY(EditDefaultsOnly, Category = "Default")
	FGameplayTag DefaultHeadTag = { DC1AppearanceTags::Appearance_Head_01 };

	UPROPERTY(EditDefaultsOnly, Category = "Default")
	FGameplayTag DefaultEyeTag = { DC1AppearanceTags::Appearance_Eye_01 };
};
