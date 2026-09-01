// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystem/DC1UIManagerSubsystem.h"

#include "UI/DC1UserWidget.h"

#include "GameFramework/PlayerController.h"

#include "Data/DA/UI/DC1UIData.h"
#include "Data/DA/UI/DC1UILoadingData.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1Helper.h"

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
    // Server 확인
    if (false == ::IsValid(GetLocalPC()))
    {
        return nullptr;
    }

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
    // Server 확인
    APlayerController* LocalPC = GetLocalPC();
    if (false == ::IsValid(LocalPC))
    {
        return;
    }

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

    UpdateInputMode(LocalPC);
}

UDC1UserWidget* UDC1UIManagerSubsystem::CreateWidgetWithTag(const FGameplayTag& InUITag)
{
    // Server 확인
    APlayerController* LocalPC = GetLocalPC();
    if (false == ::IsValid(LocalPC))
    {
        return nullptr;
    }

    TSubclassOf<UDC1UserWidget> WidgetClass = FindWidgetClass(InUITag);
    if (false == ::IsValid(WidgetClass))
    {
        return nullptr;
    }


    UDC1UserWidget* NewWidget = CreateWidget<UDC1UserWidget>(LocalPC, WidgetClass);
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
    // Server 확인
    if (false == ::IsValid(GetLocalPC()))
    {
        return;
    }

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

    if (false == IsTraveling)
    {
        RollLoadingType(InFadeType);
    }

    FadeWidget->SetFadeType(InFadeType, PendingLoadingEntry, PendingTip);
    FadeWidget->FadeIn(InDuration, InOnComplete);
}

void UDC1UIManagerSubsystem::FadeOut(float InDuration, FSimpleDelegate InOnComplete)
{
    // Server 확인
    if (false == ::IsValid(GetLocalPC()))
    {
        return;
    }

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


    IsTraveling  = true;
}

UDC1UserWidget* UDC1UIManagerSubsystem::FindWidget(const FGameplayTag& InUITag)
{
    if (TObjectPtr<UDC1UserWidget>* FoundWidgetPtr = CreatedWidget.Find(InUITag))
    {
        return FoundWidgetPtr->Get();
    }

    return nullptr;
}

APlayerController* UDC1UIManagerSubsystem::GetLocalPC() const
{
    const UGameInstance* GameInstance = GetGameInstance();
    if (false == ::IsValid(GameInstance))
    {
        return nullptr;
    }

    const UWorld* World = GetWorld();
    if (false == ::IsValid(World))
    {
        return nullptr;
    }

    return GameInstance->GetFirstLocalPlayerController(World);
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
    // Server 확인
    APlayerController* LocalPC = GetLocalPC();
    if (false == ::IsValid(LocalPC))
    {
        return nullptr;
    }

    if (false == ::IsValid(InWidget))
    {
        return nullptr;
    }

    InWidget->AddToViewport(ScreenStack.Num());
    ScreenStack.Add(InWidget);
    InWidget->Activate();

    UpdateInputMode(LocalPC);

    return InWidget;
}

UDC1UserWidget* UDC1UIManagerSubsystem::PushScreenInternal(const FGameplayTag& InUITag, TSubclassOf<UDC1UserWidget> InWidgetClass)
{
    // 레벨 전환 시점 Player 생성 확인 + Server 확인
    APlayerController* LocalPC = GetLocalPC();
    if (false == ::IsValid(LocalPC))
    {
        return nullptr;
    }

    UDC1UserWidget* NewWidget = CreateWidget<UDC1UserWidget>(LocalPC, InWidgetClass);
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

    UpdateInputMode(LocalPC);

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

void UDC1UIManagerSubsystem::UpdateInputMode(APlayerController* InLocalPC)
{
    if (ScreenStack.Num() <= 0)
    {
        FInputModeGameOnly InputMode;
        InLocalPC->SetInputMode(InputMode);
        InLocalPC->SetShowMouseCursor(false);
    }
    else
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        InLocalPC->SetInputMode(InputMode);
        InLocalPC->SetShowMouseCursor(true);
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

    const UDataTable* TipTable = DC1DataTableHelper::GetDataTable(DC1DataTableTags::DataTable_UI_LoadingTip);
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

    if (false == IsTraveling)
    {
        return;
    }

    FadeIn(0.f, PendingFadeType);

    IsTraveling = false;
}
