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
		// 부분 갱신 이벤트 구독 (성능 최적화)
		CachedViewModel->OnSlotUIUpdated.AddUObject(this, &UDC1InventorySlotsWidget::HandleSlotUpdated);
		// 레거시 전체 갱신 이벤트도 유지 (필요시)
		CachedViewModel->OnInventoryUpdated.AddUObject(this, &UDC1InventorySlotsWidget::HandleInventoryUpdated);
		RefreshAllSlots();
	}
}

void UDC1InventorySlotsWidget::NativeDestruct()
{
	if (CachedViewModel)
	{
		CachedViewModel->OnSlotUIUpdated.RemoveAll(this);
		CachedViewModel->OnInventoryUpdated.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UDC1InventorySlotsWidget::SetInventoryViewModel(UDC1InventoryViewModel* InViewModel)
{
	// 기존 ViewModel 이벤트 구독 해제
	if (CachedViewModel)
	{
		CachedViewModel->OnSlotUIUpdated.RemoveAll(this);
		CachedViewModel->OnInventoryUpdated.RemoveAll(this);
	}

	CachedViewModel = InViewModel;

	if (CachedViewModel)
	{
		// 부분 갱신 이벤트 구독 (성능 최적화)
		CachedViewModel->OnSlotUIUpdated.AddUObject(this, &UDC1InventorySlotsWidget::HandleSlotUpdated);
		// 레거시 전체 갱신 이벤트도 유지 (필요시)
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

			NewSlotWidget->InitSlot(Index);

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

	SlotWidget->SetSlotData(CachedViewModel);
}

void UDC1InventorySlotsWidget::HandleInventoryUpdated()
{
	// 레거시: 전체 슬롯 갱신 (필요시 호출)
	RefreshAllSlots();
}

void UDC1InventorySlotsWidget::HandleSlotUpdated(int32 SlotIndex)
{
	// 부분 갱신: 특정 슬롯만 업데이트 (성능 최적화)
	UpdateSlotDisplay(SlotIndex);
}

int32 UDC1InventorySlotsWidget::GetSlotIndexFromPosition(FVector2D MousePosition) const
{
	if (!SlotGrid)
	{
		return -1;
	}

	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		UDC1InventorySlotWidget* SlotWidget = SlotWidgets[i];
		if (!IsValid(SlotWidget))
		{
			continue;
		}

		FVector2D SlotSize = SlotWidget->GetDesiredSize();
		FVector2D SlotPosition = SlotWidget->GetTickSpaceGeometry().GetAbsolutePosition();

		FVector2D RelativeMousePos = MousePosition - SlotPosition;

		if (RelativeMousePos.X >= 0 && RelativeMousePos.X <= SlotSize.X &&
			RelativeMousePos.Y >= 0 && RelativeMousePos.Y <= SlotSize.Y)
		{
			return i;
		}
	}

	return -1;
}
