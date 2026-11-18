// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/PDA/DC1AssetData.h"
#include "UObject/ObjectSaveContext.h"

void UDC1AssetData::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

	// Container Clear
	AssetTagToPath.Empty();
	AssetLabelToSet.Empty();

	AssetGroupNameToSet.KeySort([](const FName& Src, const FName& Dst)
		{
			return (Src.Compare(Dst) < 0);
		});

	for (const TPair<FName, FAssetSet>& Pair : AssetGroupNameToSet)
	{
		const FAssetSet& AssetSet = Pair.Value;
		for (FAssetEntry AssetEntry : AssetSet.AssetEntries)
		{
			AssetTagToPath.Emplace(AssetEntry.AssetTag, AssetEntry.AssetPath);
			for (const FGameplayTag& Label : AssetEntry.AssetLabels)
			{
				AssetLabelToSet.FindOrAdd(Label).AssetEntries.Emplace(AssetEntry);
			}
		}
	}
}

FSoftObjectPath UDC1AssetData::GetAssetPathByTag(const FGameplayTag& AssetTag)
{
	const FSoftObjectPath AssetPath = AssetTagToPath.FindRef(AssetTag);
	if (true == AssetPath.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1AssetData :: Can't Find Asset Path from Asset Tag [%s]."), *AssetTag.ToString());
	}
	return AssetPath;
}

const FAssetSet* UDC1AssetData::GetAssetSetByLabel(const FGameplayTag& Label) const
{
	const FAssetSet* AssetSet = AssetLabelToSet.Find(Label);
	if (nullptr == AssetSet)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1AssetData :: Can't Find Asset Path from Asset Tag [%s]."), *Label.ToString());
		return nullptr;
	}

	return AssetSet;
}
