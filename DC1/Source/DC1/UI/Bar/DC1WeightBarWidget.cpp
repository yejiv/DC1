// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Bar/DC1WeightBarWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Engine/LocalPlayer.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Inventory/DC1InventoryViewModel.h"

void UDC1WeightBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ULocalPlayer* OwnerLocalPlayer = GetOwningLocalPlayer();
	if (false == ::IsValid(OwnerLocalPlayer))
	{
		return;
	}

	if (UDC1ViewModelSubsystem* VMSubsystem = OwnerLocalPlayer->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		InventoryViewModel = VMSubsystem->GetViewModel<UDC1InventoryViewModel>();
	}
}

void UDC1WeightBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
}

void UDC1WeightBarWidget::NativeDestruct()
{
	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1WeightBarWidget::BindViewModel()
{
	if (false == InventoryViewModel.IsValid())
	{
		return;
	}

	InventoryViewModel->OnInventoryUpdated.AddUObject(this, &ThisClass::Refresh);

	Refresh();
}

void UDC1WeightBarWidget::UnbindViewModel()
{
	if (false == InventoryViewModel.IsValid())
	{
		return;
	}

	InventoryViewModel->OnInventoryUpdated.RemoveAll(this);
}

void UDC1WeightBarWidget::Refresh()
{
	if (false == InventoryViewModel.IsValid())
	{
		return;
	}

	const float CurrentWeight = InventoryViewModel->GetCurrentWeight();
	const float MaxWeight = InventoryViewModel->GetMaxWeight();
	const bool bOverweight = InventoryViewModel->IsOverweight();

	ProgressBar_Weight->SetPercent(0.f < MaxWeight ? FMath::Clamp(CurrentWeight / MaxWeight, 0.f, 1.f) : 0.f);
	TextBlock_Weight->SetText(FText::FromString(FString::Printf(TEXT("%.1f / %.1f"), CurrentWeight, MaxWeight)));

	WeightBarBox->SetContentColorAndOpacity(bOverweight ? OverweightColor : DefaultColor);

	if (true == bShowOnlyWhenOverweight)
	{
		SetVisibility(bOverweight ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}
