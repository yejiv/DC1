// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/Inventory/DC1InventoryWidget.h"
#include "UI/Tab/Inventory/DC1InventorySlotsWidget.h"
#include "ViewModel/Inventory/DC1InventoryViewModel.h"

#include "UI/Item/Drag/DC1ItemDragWidget.h"

#include "Data/DataTable/DC1DataTableTypes.h"

#include "Component/Inventory/DC1InventoryTypes.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"

UDC1InventoryWidget::UDC1InventoryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDC1InventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (CachedViewModel)
	{
		HandleInventoryUpdated();
	}
}

void UDC1InventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CachedViewModel)
	{
		CachedViewModel->OnInventoryUpdated.AddUObject(this, &UDC1InventoryWidget::HandleInventoryUpdated);
	}
}

void UDC1InventoryWidget::NativeDestruct()
{
	if (CachedViewModel)
	{
		CachedViewModel->OnInventoryUpdated.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UDC1InventoryWidget::SetViewModel(UDC1InventoryViewModel* NewViewModel)
{
	if (CachedViewModel)
	{
		CachedViewModel->OnInventoryUpdated.RemoveAll(this);
	}

	CachedViewModel = NewViewModel;

	if (CachedViewModel)
	{
		if (InventorySlotsWidget)
		{
			InventorySlotsWidget->SetInventoryViewModel(CachedViewModel);
		}

		// 이벤트 구독
		CachedViewModel->OnInventoryUpdated.AddUObject(this, &UDC1InventoryWidget::HandleInventoryUpdated);
		HandleInventoryUpdated();
	}
}

void UDC1InventoryWidget::HandleInventoryUpdated()
{
	if (!CachedViewModel)
	{
		return;
	}

	if (InventoryTitle)
	{
		InventoryTitle->SetText(FText::FromString(TEXT("Inventory")));
	}
}

FReply UDC1InventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (!IsValid(InventorySlotsWidget) || !IsValid(CachedViewModel))
	{
		return FReply::Unhandled();
	}

	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	int32 ClickedSlotIndex = InventorySlotsWidget->CalculateSlotIndex(InMouseEvent.GetScreenSpacePosition());
	int32 TotalSlots = CachedViewModel->GetSlotCount();
	if (ClickedSlotIndex < 0 || ClickedSlotIndex >= TotalSlots)
	{
		return FReply::Unhandled();
	}

	if (DragSourceSlotIndex == -1)
	{
		const FDC1InventorySlotInfo& InventorySlotInfo = CachedViewModel->GetSlot(ClickedSlotIndex);
		if (InventorySlotInfo.IsEmpty())
		{
			return FReply::Unhandled();
		}

		int32 DragCount = InventorySlotInfo.StackCount;
		if (InMouseEvent.IsControlDown())
		{
			DragCount = 1;
		}
		else if (InMouseEvent.IsShiftDown())
		{
			DragCount = FMath::Max(1, InventorySlotInfo.StackCount / 2);
		}

		DragSourceSlotIndex = ClickedSlotIndex;
		DragStackCount = DragCount;
		DragItemData = CachedViewModel->GetItemData(InventorySlotInfo.ItemID);

		if (!DragItemData)
		{
			DragSourceSlotIndex = -1;
			return FReply::Unhandled();
		}

		if (!DragWidgetContainer)
		{
			DragWidgetContainer = CreateWidget<UDC1ItemDragWidget>(GetOwningPlayer());
			if (IsValid(DragWidgetContainer))
			{
				DragWidgetContainer->InitDragWidget(DragItemData, DragStackCount);
			}
		}

		return FReply::Handled();
	}
	else if (ClickedSlotIndex != DragSourceSlotIndex) // 아이템 이동
	{
		CachedViewModel->RequestMoveItem(DragSourceSlotIndex, ClickedSlotIndex, DragStackCount);

		DragSourceSlotIndex = -1;
		DragStackCount = 0;
		DragItemData = nullptr;

		if (IsValid(DragWidgetContainer))
		{
			DragWidgetContainer->RemoveFromParent();
			DragWidgetContainer = nullptr;
		}

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply UDC1InventoryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	// 첫 번째 클릭 후 마우스 이동: ItemDragWidget 위치 업데이트
	if (DragSourceSlotIndex != -1 && IsValid(DragWidgetContainer))
	{
		FVector2D MousePosition = InMouseEvent.GetScreenSpacePosition();
		DragWidgetContainer->SetPositionInViewport(MousePosition);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply UDC1InventoryWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	// 아이템이 선택된 상태에서 마우스를 놓음
	if (DragSourceSlotIndex != -1 && IsValid(CachedViewModel) && IsValid(InventorySlotsWidget))
	{
		int32 TargetSlotIndex = InventorySlotsWidget->CalculateSlotIndex(InMouseEvent.GetScreenSpacePosition());
		int32 TotalSlots = CachedViewModel->GetSlotCount();

		if (TargetSlotIndex >= 0 && TargetSlotIndex < TotalSlots && TargetSlotIndex != DragSourceSlotIndex)
		{
			CachedViewModel->RequestMoveItem(DragSourceSlotIndex, TargetSlotIndex, DragStackCount);
		}

		// 선택 상태 초기화
		DragSourceSlotIndex = -1;
		DragStackCount = 0;
		DragItemData = nullptr;

		// ItemDragWidget 제거
		if (IsValid(DragWidgetContainer))
		{
			DragWidgetContainer->RemoveFromParent();
			DragWidgetContainer = nullptr;
		}

		return FReply::Handled();
	}

	return FReply::Unhandled();
}
