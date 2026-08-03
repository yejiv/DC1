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

    for (const auto& Pair : AssetGroupNameToSet)
    {
        const FAssetSet& AssetSet = Pair.Value;
        for (FAssetEntry AssetEntry : AssetSet.AssetEntries)
        {
            FSoftObjectPath& AssetPath = AssetEntry.AssetPath;

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
    FSoftObjectPath* AssetPath = AssetTagToPath.Find(AssetTag);
    if (nullptr == AssetPath)
    {

        UE_LOG(LogTemp, Warning, TEXT("Can't Find Asset Path from Asset Tag [%s]."), *AssetTag.ToString());
        return FSoftObjectPath();
    }
    return *AssetPath;
}

const FAssetSet& UDC1AssetData::GetAssetSetByLabel(const FGameplayTag& Label)
{
    FAssetSet* AssetSet = AssetLabelToSet.Find(Label);
    ensureAlwaysMsgf(AssetSet, TEXT("Can't Find Asset Path from Asset Tag [%s]."), *Label.ToString());

    return *(AssetSet);
}

const FAssetSet* UDC1AssetData::FindAssetSetByLabel(const FGameplayTag& Label) const
{
    return AssetLabelToSet.Find(Label);
}
