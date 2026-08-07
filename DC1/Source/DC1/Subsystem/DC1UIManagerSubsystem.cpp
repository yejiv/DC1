// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystem/DC1UIManagerSubsystem.h"

#include "UI/DC1UserWidget.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

#include "Data/DA/UI/DC1UIData.h"
#include "Data/DA/UI/DC1UILoadingData.h"
#include "Data/DA/DataTable/DC1DataTableData.h"
#include "Data/DataTable/DC1DataTableTypes.h"

#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/UI/DC1GameplayTag_UI.h"

void UDC1UIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnPostLoadMap);
}

void UDC1UIManagerSubsystem::Deinitialize()
{
    Super::Deinitialize();

    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
}

UDC1UserWidget* UDC1UIManagerSubsystem::PushScreen(const FGameplayTag& UITag, bool IsPop)
{
    TSubclassOf<UDC1UserWidget> WidgetClass = FindWidgetClass(UITag);
    if (false == ::IsValid(WidgetClass))
    {
        return nullptr;
    }

    if (true == IsPop)
    {
        PopScreen();
    }

    return PushScreenInternal(WidgetClass);
}

void UDC1UIManagerSubsystem::PopScreen()
{
    if (0 >= ScreenStack.Num())
    {
        return;
    }

    UDC1UserWidget* TopWidget = ScreenStack.Pop();
    if (true == ::IsValid(TopWidget))
    {
        TopWidget->Deactivate();
        TopWidget->RemoveFromParent();
    }
    
    UpdateInputMode();
}

void UDC1UIManagerSubsystem::FadeIn(float InDuration, EDC1FadeType FadeType, FSimpleDelegate OnComplete)
{
    if (false == ::IsValid(FadeWidget))
    {
        FadeWidget = Cast<UDC1FadeWidget>(CreateOverlay(DC1UITags::UI_Fade, FadeZOrder));

        if (false == ::IsValid(FadeWidget))
        {
            OnComplete.ExecuteIfBound();
            return;
        }
    }

    if (FadeWidget->IsFading())
    {
        return;
    }

    if (false == bIsTraveling)
    {
        RollLoadingType(FadeType);
    }

    FadeWidget->SetFadeType(FadeType, PendingLoadingEntry, PendingTip);
    FadeWidget->FadeIn(InDuration, OnComplete);
}

void UDC1UIManagerSubsystem::FadeOut(float InDuration, FSimpleDelegate OnComplete)
{
    if (false == ::IsValid(FadeWidget))
    {
        OnComplete.ExecuteIfBound();
        return;
    }

    if (FadeWidget->IsFading())
    {
        return;
    }

    FadeWidget->FadeOut(InDuration, OnComplete);
}

void UDC1UIManagerSubsystem::BeginLoadingTransition(float InDuration, FSimpleDelegate OnReadyToTravel)
{
    FadeIn(InDuration, EDC1FadeType::Loading, FSimpleDelegate::CreateWeakLambda(this, [this, OnReadyToTravel]
        {
            while (0 < ScreenStack.Num())
            {
                PopScreen();
            }

            OnReadyToTravel.ExecuteIfBound();
        }));


    bIsTraveling = true;
}

UDC1UserWidget* UDC1UIManagerSubsystem::CreateOverlay(const FGameplayTag& UITag, int32 ZOrder)
{
    TSubclassOf<UDC1UserWidget> WidgetClass = FindWidgetClass(UITag);
    if (false == ::IsValid(WidgetClass))
    {
        return nullptr;
    }

    UDC1UserWidget* NewWidget = CreateWidget<UDC1UserWidget>(GetGameInstance(), WidgetClass);
    if (false == ::IsValid(NewWidget))
    {
        return nullptr;
    }

    NewWidget->AddToViewport(ZOrder);
    return NewWidget;
}

UDC1UserWidget* UDC1UIManagerSubsystem::PushScreenInternal(TSubclassOf<UDC1UserWidget> WidgetClass)
{
    if (false == ::IsValid(WidgetClass))
    {
        return nullptr;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (false == ::IsValid(PC))
    {
        return nullptr;
    }

    UDC1UserWidget* NewWidget = CreateWidget<UDC1UserWidget>(PC, WidgetClass);
    if (false == ::IsValid(NewWidget))
    {
        return nullptr;
    }

    NewWidget->AddToViewport(ScreenStack.Num());
    ScreenStack.Add(NewWidget);
    NewWidget->Activate();

    UpdateInputMode();

    return NewWidget;
}

TSubclassOf<UDC1UserWidget> UDC1UIManagerSubsystem::FindWidgetClass(const FGameplayTag& UITag)
{
    if (false == ::IsValid(CachedUIData))
    {
        CachedUIData = UDC1AssetManager::GetAssetByTag<UDC1UIData>(DC1AssetTags::Asset_UI_WidgetData);
        if(false == ::IsValid(CachedUIData))
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to Caching UIData"));
            return nullptr;
        }
    }

    return CachedUIData->FindUserWidgetClassByTag(UITag);
}

void UDC1UIManagerSubsystem::UpdateInputMode()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (false == ::IsValid(PC))
    {
        return;
    }

    if (0 >= ScreenStack.Num())
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(false);
    }
    else
    {
        FInputModeUIOnly InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(true);
    }
}

void UDC1UIManagerSubsystem::RollLoadingType(EDC1FadeType InType)
{
    PendingFadeType = InType;

    if (EDC1FadeType::Default == PendingFadeType)
    {
        PendingLoadingEntry = nullptr;
        PendingTip = nullptr;
        return;
    }

    // EDC1FadeType::Spinner 
    const UDC1UILoadingData* LoadingData = UDC1AssetManager::GetAssetByTag<UDC1UILoadingData>(DC1AssetTags::Asset_UI_LoadingData);
    if (true == ::IsValid(LoadingData))
    {
        const FUILoadingEntry* CachedLoadingEntry = LoadingData->GetRandomSprites();
        if (nullptr != CachedLoadingEntry)
        {
            PendingLoadingEntry = CachedLoadingEntry;
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to Find LoadingData"));
    }

    if (EDC1FadeType::Spinner == PendingFadeType)
    {
        PendingTip = nullptr;
        return;
    }

    // EDC1FadeType::Loading
    const UDC1DataTableData* DataTable = UDC1AssetManager::GetAssetByTag<UDC1DataTableData>(DC1AssetTags::Asset_DataTableData);
    if (true == ::IsValid(DataTable))
    {
        UDataTable* TipTable = DataTable->GetDataTable(DC1DataTableTags::DataTable_UI_LoadingTip);
        if (true == ::IsValid(TipTable))
        {
            TArray<FName> RowNames = TipTable->GetRowNames();
            if (false == RowNames.IsEmpty())
            {
                const FName Picked = RowNames[FMath::RandRange(0, RowNames.Num() - 1)];
                if (const FDC1LoadingTipRow* Row = TipTable->FindRow<FDC1LoadingTipRow>(Picked, TEXT("RollLoadingContent")))
                {
                    PendingTip = &(Row->Tip);
                }
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to Find DataTableData"));
    }

}

void UDC1UIManagerSubsystem::OnPostLoadMap(UWorld* LoadedWorld)
{
    ScreenStack.Empty();
    FadeWidget = nullptr;

    if (false == bIsTraveling)
    {
        return;
    }

    FadeIn(0.f, PendingFadeType);

    bIsTraveling = false;
}
