// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/Inventory/DC1InventorySlotsWidget.h"
#include "UI/Tab/Inventory/DC1InventorySlotWidget.h"
#include "ViewModel/Inventory/DC1InventoryViewModel.h"
#include "Components/ScrollBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

UDC1InventorySlotsWidget::UDC1InventorySlotsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDC1InventorySlotsWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (SlotWidgets.Num() == 0)
	{
		CreateSlotWidgets(DefaultSlotCapacity);
	}
}

void UDC1InventorySlotsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CachedViewModel)
	{
		CachedViewModel->OnInventoryUpdated.AddUObject(this, &UDC1InventorySlotsWidget::HandleInventoryUpdated);
		RefreshAllSlots();
	}
}

void UDC1InventorySlotsWidget::NativeDestruct()
{
	if (CachedViewModel)
	{
		CachedViewModel->OnInventoryUpdated.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UDC1InventorySlotsWidget::SetInventoryViewModel(UDC1InventoryViewModel* InViewModel)
{
	// 기존 ViewModel 이벤트 구독 해제
	if (CachedViewModel)
	{
		CachedViewModel->OnInventoryUpdated.RemoveAll(this);
	}

	CachedViewModel = InViewModel;

	if (CachedViewModel)
	{
		CachedViewModel->OnInventoryUpdated.AddUObject(this, &UDC1InventorySlotsWidget::HandleInventoryUpdated);
		RefreshAllSlots();
	}
}

void UDC1InventorySlotsWidget::RefreshSlotCapacity(int32 NewTotalSlots)
{
	CreateSlotWidgets(NewTotalSlots);
}

void UDC1InventorySlotsWidget::RefreshAllSlots()
{
	if (!CachedViewModel)
	{
		return;
	}

	const int32 SlotCount = CachedViewModel->GetSlotCount();
	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		UpdateSlotDisplay(i);
	}
}

void UDC1InventorySlotsWidget::CreateSlotWidgets(int32 TotalSlots)
{
	if (!SlotGrid)
	{
		UE_LOG(LogTemp, Error, TEXT("[UDC1InventorySlotsWidget] SlotGrid is NULL"));
		return;
	}

	if (!SlotWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UDC1InventorySlotsWidget] SlotWidgetClass is not assigned in Blueprint Details."));
		return;
	}

	SlotGrid->ClearChildren();
	SlotWidgets.Empty();
	SlotWidgets.Reserve(TotalSlots);

	for (int32 Index = 0; Index < TotalSlots; ++Index)
	{
		UDC1InventorySlotWidget* NewSlotWidget = CreateWidget<UDC1InventorySlotWidget>(this, SlotWidgetClass);
		if (NewSlotWidget)
		{
			const int32 Row = Index / InventoryColumns;
			const int32 Column = Index % InventoryColumns;

			UUniformGridSlot* GridSlot = SlotGrid->AddChildToUniformGrid(NewSlotWidget, Row, Column);
			if (GridSlot)
			{
				GridSlot->SetHorizontalAlignment(HAlign_Center);
				GridSlot->SetVerticalAlignment(VAlign_Center);
			}

			SlotWidgets.Add(NewSlotWidget);
		}
	}
}

void UDC1InventorySlotsWidget::UpdateSlotDisplay(int32 SlotIndex)
{
	if (!CachedViewModel || !SlotWidgets.IsValidIndex(SlotIndex))
	{
		return;
	}

	UDC1InventorySlotWidget* SlotWidget = SlotWidgets[SlotIndex];
	if (!SlotWidget)
	{
		return;
	}

	SlotWidget->SetSlotData(CachedViewModel, SlotIndex);
}

void UDC1InventorySlotsWidget::HandleInventoryUpdated()
{
	RefreshAllSlots();
}
