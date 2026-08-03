// Fill out your copyright notice in the Description page of Project Settings.


#include "System/DC1AssetManager.h"

UDC1AssetManager::UDC1AssetManager()
    : Super()
{
}

UDC1AssetManager& UDC1AssetManager::Get()
{
    if (UDC1AssetManager* SingleTon = Cast<UDC1AssetManager>(GEngine->AssetManager))
    {
        return *SingleTon;
    }

    return *(NewObject<UDC1AssetManager>());
}

void UDC1AssetManager::LoadAssetsToLabel(const FGameplayTag& LabelTag, FAsyncLabelLoadCompletedDelegate CompletedDelegate)
{
    if (LoadedPrimaryAssetData)
    {
        LoadAsyncByLabel(LabelTag, MoveTemp(CompletedDelegate));
    }
    else
    {
        UDC1AssetData* AssetData = nullptr;
        FPrimaryAssetType PrimaryAssetType(UDC1AssetData::StaticClass()->GetFName());
        TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(PrimaryAssetType);

        if (Handle.IsValid())
        {
            Handle->WaitUntilComplete(0.f, false);
            AssetData = Cast<UDC1AssetData>(Handle->GetLoadedAsset());
        }

        if (AssetData)
        {
            LoadedPrimaryAssetData = AssetData;
            LoadAsyncByLabel(LabelTag, MoveTemp(CompletedDelegate));
        }
        else
        {
            UE_LOG(LogWindows, Fatal, TEXT("Failed to load AssetData asset type [%s]."), *PrimaryAssetType.ToString());
        }
    }
}

void UDC1AssetManager::LoadAsyncByPath(const FSoftObjectPath& AssetPath, const FGameplayTag& AssetTag, FAsyncLoadCompletedDelegate CompletedDelegate)
{
    if (false == UAssetManager::IsInitialized())
    {
        UE_LOG(LogTemp, Error, TEXT("AssetManager must be initialized"));
        return;
    }

    if (true == AssetPath.IsValid())
    {
        if (UObject* LoadedAsset = AssetPath.ResolveObject())
        {
            const FName& AssetName = AssetPath.GetAssetFName();
            Get().AddLoadedAsset(AssetName, AssetTag, LoadedAsset);
        }
        else
        {
            TArray<FSoftObjectPath> AssetPaths;
            AssetPaths.Add(AssetPath);

            TSharedPtr<FStreamableHandle> Handle = GetStreamableManager().RequestAsyncLoad(AssetPaths);

            Handle->BindCompleteDelegate(FStreamableDelegate::CreateLambda([AssetName = AssetPath.GetAssetFName(), AssetTag, AssetPath, CompleteDelegate = MoveTemp(CompletedDelegate)]()
                {
                    UObject* LoadedAsset = AssetPath.ResolveObject();
                    Get().AddLoadedAsset(AssetName, AssetTag, LoadedAsset);
                    if (CompleteDelegate.IsBound())
                        CompleteDelegate.Execute(AssetName, LoadedAsset);
                }));
        }
    }
}

void UDC1AssetManager::LoadAsyncByTag(const FGameplayTag& AssetTag, FAsyncLoadCompletedDelegate CompletedDelegate)
{
    if (false == UAssetManager::IsInitialized())
    {
        UE_LOG(LogTemp, Error, TEXT("AssetManager must be initialized"));
        return;
    }

    UDC1AssetData* AssetData = Get().LoadedPrimaryAssetData;
    if (false == ::IsValid(AssetData))
    {
        UE_LOG(LogWindows, Fatal, TEXT("Can't Find AssetData"));
        return;
    }

    const FSoftObjectPath& AssetPath = AssetData->GetAssetPathByTag(AssetTag);
    LoadAsyncByPath(AssetPath, AssetTag, CompletedDelegate);
}

void UDC1AssetManager::LoadAsyncByLabel(const FGameplayTag& LabelTag, FAsyncLabelLoadCompletedDelegate CompletedDelegate)
{
    if (false == UAssetManager::IsInitialized())
    {
        UE_LOG(LogTemp, Error, TEXT("AssetManager must be initialized"));
        return;
    }

    TArray<FSoftObjectPath> PathsToLoad;
    TArray<FAssetEntry> EntriesToLoad;

    const UDC1AssetData* AssetData = Get().LoadedPrimaryAssetData;
    if (false == ::IsValid(AssetData))
    {
        UE_LOG(LogWindows, Fatal, TEXT("Can't Find AssetData"));
        return;
    }

    const FAssetSet* AssetSet = AssetData->FindAssetSetByLabel(LabelTag);
    if (nullptr == AssetSet)
    {
        UE_LOG(LogWindows, Fatal, TEXT("Can't Find AssetSet"));
        return;
    }

    for (const FAssetEntry& Entry : AssetSet->AssetEntries)
    {
        if (true == Entry.AssetPath.IsValid())
        {
            PathsToLoad.Add(Entry.AssetPath);
            EntriesToLoad.Add(Entry);
        }
    }

    if (PathsToLoad.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("None Asset to Label : [%s]."), *LabelTag.ToString());
        return;
    }

    UDC1AssetManager* AssetManager = &Get();

    AssetManager->ActiveLoadHandle = GetStreamableManager().RequestAsyncLoad(
        PathsToLoad,
        FStreamableDelegate::CreateLambda([AssetManager, EntriesToLoad, CompletedDelegate]()
            {
                for (const FAssetEntry& Entry : EntriesToLoad)
                {
                    if (UObject* Loaded = Entry.AssetPath.ResolveObject())
                    {
                        const FName& AssetName = Entry.AssetPath.GetAssetFName();
                        AssetManager->AddLoadedAsset(AssetName, Entry.AssetTag, Loaded);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Failed to resolve asset [%s]"), *Entry.AssetPath.ToString());
                    }
                }

                if (CompletedDelegate.IsBound())
                {
                    CompletedDelegate.Execute();
                }

                AssetManager->ActiveLoadHandle.Reset();
            })
    );
}

void UDC1AssetManager::ReleaseByPath(const FSoftObjectPath& AssetPath)
{
    if (false == UAssetManager::IsInitialized())
    {
        UE_LOG(LogTemp, Error, TEXT("AssetManager must be initialized"));
        return;
    }

    FName AssetName = AssetPath.GetAssetFName();
    UDC1AssetManager& AssetManager = Get();

    FGameplayTag* FoundTag = AssetManager.NameToTag.Find(AssetName);
    if (nullptr != FoundTag && true == FoundTag->IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("ReleaseByPath: None Loaded Asset, Path - [%s]"), *AssetPath.ToString());
        return;
    }

    AssetManager.TagToLoadedDataAsset.Remove(*FoundTag);
    AssetManager.NameToTag.Remove(AssetName);
}

void UDC1AssetManager::ReleaseByTag(const FGameplayTag& AssetTag)
{
    if (false == UAssetManager::IsInitialized())
    {
        UE_LOG(LogTemp, Error, TEXT("AssetManager must be initialized"));
        return;
    }

    UDC1AssetManager& AssetManager = Get();

    if (false == AssetManager.TagToLoadedDataAsset.Contains(AssetTag))
    {
        UE_LOG(LogTemp, Warning, TEXT("ReleaseByTag: None Loaded Asset, Tag - [%s]"), *AssetTag.ToString());
        return;
    }

    AssetManager.TagToLoadedDataAsset.Remove(AssetTag);

    // NameToTag는 Value(Tag)로 찾아야 하므로 순회 삭제
    for (auto It = AssetManager.NameToTag.CreateIterator(); It; ++It)
    {
        if (It.Value() == AssetTag)
        {
            It.RemoveCurrent();
            break;  // Tag - Name 1:1 매핑이므로 찾으면 즉시 종료
        }
    }
}

void UDC1AssetManager::ReleaseByLabel(const FGameplayTag& Label)
{
    if (false == UAssetManager::IsInitialized())
    {
        UE_LOG(LogTemp, Error, TEXT("AssetManager must be initialized"));
        return;
    }

    UDC1AssetManager& AssetManager = Get();
    UDC1AssetData* LoadedAssetData = AssetManager.LoadedPrimaryAssetData;
    if (false == ::IsValid(LoadedAssetData))
    {
        UE_LOG(LogWindows, Fatal, TEXT("Can't Find AssetData"));
        return;
    }

    // 제거할 Tag 목록 수집
    TArray<FGameplayTag> TagsToRemove;

    const FAssetSet* AssetSet = LoadedAssetData->FindAssetSetByLabel(Label);
    if (nullptr == AssetSet)
    {
        UE_LOG(LogWindows, Fatal, TEXT("Can't Find AssetSet"));
        return;
    }
    for (const FAssetEntry& AssetEntry : AssetSet->AssetEntries)
    {
        TagsToRemove.Add(AssetEntry.AssetTag);
    }
    // TagToLoadedAsset 제거
    for (const FGameplayTag& Tag : TagsToRemove)
    {
        AssetManager.TagToLoadedDataAsset.Remove(Tag);
    }

    // NameToTag에서 Value가 일치하는 항목 순회 삭제
    for (auto It = AssetManager.NameToTag.CreateIterator(); It; ++It)
    {
        if (TagsToRemove.Contains(It.Value()))
        {
            It.RemoveCurrent();
        }
    }
}

void UDC1AssetManager::ReleaseAll()
{
    if (false == UAssetManager::IsInitialized())
    {
        UE_LOG(LogTemp, Error, TEXT("AssetManager must be initialized"));
        return;
    }

    UDC1AssetManager& AssetManager = Get();

    AssetManager.LoadedPrimaryAssetData = nullptr;
    AssetManager.TagToLoadedDataAsset.Reset();
    AssetManager.NameToTag.Reset();
}

TWeakPtr<FStreamableHandle> UDC1AssetManager::GetActiveLoadHandle()
{
    const TSharedPtr<FStreamableHandle>& Handle = Get().ActiveLoadHandle;
    if (Handle.IsValid())
        return TWeakPtr<FStreamableHandle>(Handle);

    UE_LOG(LogTemp, Warning, TEXT("GetActiveLoadHandle: None ActiveLoadHandle, Check After the LoadAsyncByLabel"));
    return TWeakPtr<FStreamableHandle>();
}

void UDC1AssetManager::AddLoadedAsset(const FName& AssetName, const FGameplayTag& AssetTag, const UObject* Asset)
{
    if (nullptr != Asset && true == AssetTag.IsValid())
    {
        //FScopeLock LoadedAssetsLock(&LoadedAssetsCritical);

        if (false == NameToTag.Contains(AssetName))
        {
            NameToTag.Emplace(AssetName, AssetTag);
        }

        if (false == TagToLoadedDataAsset.Contains(AssetTag))
        {
            TagToLoadedDataAsset.Emplace(AssetTag, Asset);
        }
    }
}
