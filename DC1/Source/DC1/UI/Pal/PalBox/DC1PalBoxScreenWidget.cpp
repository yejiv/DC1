// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Pal/PalBox/DC1PalBoxScreenWidget.h"

#include "Subsystem/DC1ViewModelSubsystem.h"
#include "UI/Pal/PalBox/DC1PalBoxDetailWidget.h"
#include "UI/Pal/PalBox/DC1PalBoxSlotWidget.h"
#include "ViewModel/Pal/DC1PalBoxViewModel.h"

#include "Components/PanelWidget.h"

void UDC1PalBoxScreenWidget::HandleInputKey(const FKey& InKey)
{
	if (InKey != CloseKey)
	{
		return;
	}

	PopScreen();
}

void UDC1PalBoxScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
}

void UDC1PalBoxScreenWidget::NativeDestruct()
{
	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1PalBoxScreenWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// 열 때는 NoData
	HoveredSlot = INDEX_NONE;
	RefreshSlots();
	RefreshDetail();
}

void UDC1PalBoxScreenWidget::BindViewModel()
{
	UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>();
	if (nullptr == VMSubsystem)
	{
		return;
	}

	PalBoxViewModel = VMSubsystem->GetViewModel<UDC1PalBoxViewModel>();
	if (false == PalBoxViewModel.IsValid())
	{
		return;
	}

	PalBoxViewModel->OnPartyChanged.AddUObject(this, &ThisClass::HandlePartyChanged);

	CreateSlots();
	HandlePartyChanged();
}

void UDC1PalBoxScreenWidget::UnbindViewModel()
{
	if (false == PalBoxViewModel.IsValid())
	{
		return;
	}

	PalBoxViewModel->OnPartyChanged.RemoveAll(this);
	PalBoxViewModel.Reset();
}

void UDC1PalBoxScreenWidget::CreateSlots()
{
	const int32 Capacity = PalBoxViewModel->GetCapacity();
	if (SlotWidgets.Num() == Capacity || nullptr == SlotWidgetClass)
	{
		return;
	}

	SlotContainer->ClearChildren();
	SlotWidgets.Reset();

	for (int32 Index = 0; Index < Capacity; ++Index)
	{
		UDC1PalBoxSlotWidget* SlotWidget = CreateWidget<UDC1PalBoxSlotWidget>(this, SlotWidgetClass);
		if (nullptr == SlotWidget)
		{
			continue;
		}

		SlotWidget->SetSlotIndex(Index);
		SlotWidget->OnSlotHovered.AddUObject(this, &ThisClass::HandleSlotHovered);

		SlotContainer->AddChild(SlotWidget);
		SlotWidgets.Add(SlotWidget);
	}
}

void UDC1PalBoxScreenWidget::RefreshSlots()
{
	if (false == PalBoxViewModel.IsValid())
	{
		return;
	}

	for (int32 Index = 0; Index < SlotWidgets.Num(); ++Index)
	{
		FDC1PalBoxSlotInfo SlotInfo;
		if (true == PalBoxViewModel->GetSlotInfo(Index, SlotInfo))
		{
			SlotWidgets[Index]->SetSlotInfo(SlotInfo);
		}
		else
		{
			SlotWidgets[Index]->SetEmpty();
		}
	}
}

void UDC1PalBoxScreenWidget::RefreshDetail()
{
	FDC1PalBoxDetailInfo DetailInfo;
	if (true == PalBoxViewModel.IsValid() && true == PalBoxViewModel->GetDetailInfo(HoveredSlot, DetailInfo))
	{
		Detail->SetDetailInfo(DetailInfo);
	}
	else
	{
		Detail->ShowNoData();
	}
}

void UDC1PalBoxScreenWidget::HandlePartyChanged()
{
	RefreshSlots();
	RefreshDetail();
}

void UDC1PalBoxScreenWidget::HandleSlotHovered(int32 InSlotIndex)
{
	HoveredSlot = InSlotIndex;
	RefreshDetail();
}
