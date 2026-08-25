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

UDC1UserWidget* UDC1UIManagerSubsystem::PushScreen(const FGameplayTag& InUITag, bool InIsPop)
{
    bool IsCreated = false;

    TObjectPtr<UDC1UserWidget>* FoundWidget = CreatedWidget.Find(InUITag);
    if (nullptr != FoundWidget)
    {
        IsCreated = true;
    }

    TSubclassOf<UDC1UserWidget> WidgetClass = nullptr;
    if (false == IsCreated)
    {
        WidgetClass = FindWidgetClass(InUITag);
        if (false == ::IsValid(WidgetClass))
        {
            return nullptr;
        }
    }

    if (true == InIsPop)
    {
        PopScreen();
    }

    return IsCreated ? PushScreenInternal(FoundWidget->Get()) : PushScreenInternal(InUITag, WidgetClass);
}

void UDC1UIManagerSubsystem::PopScreen()
{
    if (ScreenStack.Num() <= 0)
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

UDC1UserWidget* UDC1UIManagerSubsystem::CreateWidgetWithTag(const FGameplayTag& InUITag)
{
    TSubclassOf<UDC1UserWidget> WidgetClass = FindWidgetClass(InUITag);
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
    if (true == ::IsValid(NewWidget))
    {
        CreatedWidget.Add(InUITag, NewWidget);
    }
    else
    {
        return nullptr;
    }

    return NewWidget;
}

void UDC1UIManagerSubsystem::FadeIn(float InDuration, EDC1FadeType InFadeType, FSimpleDelegate InOnComplete)
{
    if (false == ::IsValid(FadeWidget))
    {
        FadeWidget = Cast<UDC1FadeWidget>(CreateOverlay(DC1UITags::UI_Fade, FadeZOrder));

        if (false == ::IsValid(FadeWidget))
        {
            InOnComplete.ExecuteIfBound();
            return;
        }
    }

    if (FadeWidget->IsFading())
    {
        return;
    }

    if (false == bIsTraveling)
    {
        RollLoadingType(InFadeType);
    }

    FadeWidget->SetFadeType(InFadeType, PendingLoadingEntry, PendingTip);
    FadeWidget->FadeIn(InDuration, InOnComplete);
}

void UDC1UIManagerSubsystem::FadeOut(float InDuration, FSimpleDelegate InOnComplete)
{
    if (false == ::IsValid(FadeWidget))
    {
        InOnComplete.ExecuteIfBound();
        return;
    }

    if (FadeWidget->IsFading())
    {
        return;
    }

    FadeWidget->FadeOut(InDuration, InOnComplete);
}

void UDC1UIManagerSubsystem::RouteInputKey(const FKey& InKey)
{
    if (ScreenStack.Num() <= 0)
    {
        return;
    }

    UDC1UserWidget* TopWidget = ScreenStack.Top();
    if (true == ::IsValid(TopWidget))
    {
        TopWidget->HandleInputKey(InKey);
    }
}

void UDC1UIManagerSubsystem::BeginLoadingTransition(float InDuration, FSimpleDelegate InOnReadyToTravel)
{
    FadeIn(InDuration, EDC1FadeType::Loading, FSimpleDelegate::CreateWeakLambda(this, [this, InOnReadyToTravel]
        {
            while (ScreenStack.Num() > 0)
            {
                PopScreen();
            }

            CreatedWidget.Empty();

            InOnReadyToTravel.ExecuteIfBound();
        }));


    bIsTraveling = true;
}

UDC1UserWidget* UDC1UIManagerSubsystem::FindWidget(const FGameplayTag& InUITag)
{
    if (TObjectPtr<UDC1UserWidget>* FoundWidgetPtr = CreatedWidget.Find(InUITag))
    {
        return FoundWidgetPtr->Get();
    }

    return nullptr;
}

UDC1UserWidget* UDC1UIManagerSubsystem::CreateOverlay(const FGameplayTag& InUITag, int32 InZOrder)
{
    TSubclassOf<UDC1UserWidget> WidgetClass = FindWidgetClass(InUITag);
    if (false == ::IsValid(WidgetClass))
    {
        return nullptr;
    }

    UDC1UserWidget* NewWidget = CreateWidget<UDC1UserWidget>(GetGameInstance(), WidgetClass);
    if (false == ::IsValid(NewWidget))
    {
        return nullptr;
    }

    NewWidget->AddToViewport(InZOrder);
    return NewWidget;
}

UDC1UserWidget* UDC1UIManagerSubsystem::PushScreenInternal(UDC1UserWidget* InWidget)
{
    if (false == ::IsValid(InWidget))
    {
        return nullptr;
    }

    InWidget->AddToViewport(ScreenStack.Num());
    ScreenStack.Add(InWidget);
    InWidget->Activate();

    UpdateInputMode();

    return InWidget;
}

UDC1UserWidget* UDC1UIManagerSubsystem::PushScreenInternal(const FGameplayTag& InUITag, TSubclassOf<UDC1UserWidget> InWidgetClass)
{
    // 레벨 전환 시점 Player 생성 확인
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (false == ::IsValid(PC))
    {
        return nullptr;
    }

    UDC1UserWidget* NewWidget = CreateWidget<UDC1UserWidget>(PC, InWidgetClass);
    if (true == ::IsValid(NewWidget))
    {
        CreatedWidget.Add(InUITag, NewWidget);
    }
    else
    {
        return nullptr;
    }

    NewWidget->AddToViewport(ScreenStack.Num());
    ScreenStack.Add(NewWidget);
    NewWidget->Activate();

    UpdateInputMode();

    return NewWidget;
}

TSubclassOf<UDC1UserWidget> UDC1UIManagerSubsystem::FindWidgetClass(const FGameplayTag& InUITag)
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

    return CachedUIData->FindUserWidgetClassByTag(InUITag);
}

void UDC1UIManagerSubsystem::UpdateInputMode()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (false == ::IsValid(PC))
    {
        return;
    }

    if (ScreenStack.Num() <= 0)
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(false);
    }
    else
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->SetShowMouseCursor(true);
    }

    OnScreenStackChanged.Broadcast();
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

void UDC1UIManagerSubsystem::OnPostLoadMap(UWorld* InLoadedWorld)
{
    ScreenStack.Empty();
    FadeWidget = nullptr;

    CreatedWidget.Empty();

    if (false == bIsTraveling)
    {
        return;
    }

    FadeIn(0.f, PendingFadeType);

    bIsTraveling = false;
}
