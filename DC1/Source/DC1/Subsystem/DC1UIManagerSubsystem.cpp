// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystem/DC1UIManagerSubsystem.h"

#include "UI/DC1UserWidget.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

#include "Data/DA/UI/DC1UIData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

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