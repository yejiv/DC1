// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DC1AssetData.generated.h"

USTRUCT()
struct FAssetEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag AssetTag = { FGameplayTag::EmptyTag };

	UPROPERTY(EditDefaultsOnly)
	FSoftObjectPath AssetPath;

	UPROPERTY(EditDefaultsOnly)
	TArray<FGameplayTag> AssetLabels;
};

USTRUCT()
struct FAssetSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FAssetEntry> AssetEntries;
};

UCLASS()
class DC1_API UDC1AssetData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual void		PreSave(FObjectPreSaveContext ObjectSaveContext) override;

public:
	FSoftObjectPath		GetAssetPathByTag(const FGameplayTag& AssetTag);
	const FAssetSet*	GetAssetSetByLabel(const FGameplayTag& Label) const;

private:
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, FAssetSet>				AssetGroupNameToSet;

	UPROPERTY()
	TMap<FGameplayTag, FSoftObjectPath> AssetTagToPath;

	UPROPERTY()
	TMap<FGameplayTag, FAssetSet>		AssetLabelToSet;
};
