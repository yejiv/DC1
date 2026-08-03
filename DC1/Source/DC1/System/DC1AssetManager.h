// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/AssetManager.h"
#include "GameplayTagContainer.h"
#include "Data/PDA/DC1AssetData.h"
#include "DC1AssetManager.generated.h"

DECLARE_DELEGATE_TwoParams(FAsyncLoadCompletedDelegate, const FName&, UObject*);
DECLARE_DELEGATE(FAsyncLabelLoadCompletedDelegate);

UCLASS()
class DC1_API UDC1AssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	UDC1AssetManager();

	static UDC1AssetManager& Get();

	template<typename AssetType>
	static AssetType*	GetAssetByTag(const FGameplayTag& AssetTag);

public:
	void				LoadAssetsToLabel(const FGameplayTag& LabelTag, FAsyncLabelLoadCompletedDelegate CompletedDelegate = {});
	
	static void			LoadAsyncByPath(const FSoftObjectPath& AssetPath, const FGameplayTag& AssetTag, FAsyncLoadCompletedDelegate CompletedDelegate = FAsyncLoadCompletedDelegate());
	static void			LoadAsyncByTag(const FGameplayTag& AssetTag, FAsyncLoadCompletedDelegate CompletedDelegate = FAsyncLoadCompletedDelegate());
	static void			LoadAsyncByLabel(const FGameplayTag& LabelTag, FAsyncLabelLoadCompletedDelegate CompletedDelegate = FAsyncLabelLoadCompletedDelegate());

	static void			ReleaseByPath(const FSoftObjectPath& AssetPath);
	static void			ReleaseByTag(const FGameplayTag& AssetTag);
	static void			ReleaseByLabel(const FGameplayTag& Label);
	static void			ReleaseAll();

	static TWeakPtr<FStreamableHandle> GetActiveLoadHandle();

private:
	void				AddLoadedAsset(const FName& AssetName, const FGameplayTag& AssetTag, const UObject* Asset);

private:
	// Cached AssetData
	UPROPERTY()
	TObjectPtr<UDC1AssetData> LoadedPrimaryAssetData;

	UPROPERTY()
	TMap<FName, FGameplayTag> NameToTag;

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<const UObject>> TagToLoadedDataAsset;

	TSharedPtr<FStreamableHandle> ActiveLoadHandle;
};

template<typename AssetType>
inline AssetType* UDC1AssetManager::GetAssetByTag(const FGameplayTag& AssetTag)
{
	UDC1AssetData* AssetData = Get().LoadedPrimaryAssetData;
	if (false == ::IsValid(AssetData))
	{
		return nullptr;
	}

	AssetType* LoadedAsset = nullptr;
	const FSoftObjectPath AssetPath = AssetData->GetAssetPathByTag(AssetTag);
	if (true == AssetPath.IsValid())
	{
		LoadedAsset = Cast<AssetType>(AssetPath.ResolveObject());
		if (false == ::IsValid(LoadedAsset))
		{
			UE_LOG(LogTemp, Warning, TEXT("Attempted sync loading because asset hadn't loaded yet [%s]."), *AssetTag.ToString());
			LoadedAsset = Cast<AssetType>(AssetPath.TryLoad());
		}
	}

	return LoadedAsset;
}
