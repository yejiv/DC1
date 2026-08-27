// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/Inventory/DC1InventoryWidget.h"
#include "UI/Tab/Inventory/DC1InventorySlotsWidget.h"
#include "UI/Item/Drag/DC1DragDropOperation.h"
#include "ViewModel/Inventory/DC1InventoryViewModel.h"
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

bool UDC1InventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (!IsValid(InOperation) || !IsValid(InventorySlotsWidget) || !IsValid(CachedViewModel))
	{
		return false;
	}

	UDC1DragDropOperation* DC1DragOp = Cast<UDC1DragDropOperation>(InOperation);
	if (!DC1DragOp)
	{
		return false;
	}

	const int32 SourceSlotIndex = DC1DragOp->GetSourceSlot();
	if (SourceSlotIndex < 0)
	{
		return false;
	}

	FVector2D MousePosition = InDragDropEvent.GetScreenSpacePosition();
	FVector2D SlotGridPosition = InventorySlotsWidget->GetTickSpaceGeometry().GetAbsolutePosition();
	FVector2D RelativeMousePos = MousePosition - SlotGridPosition;

	int32 TargetSlotIndex = InventorySlotsWidget->GetSlotIndexFromPosition(RelativeMousePos);
	if (TargetSlotIndex < 0 || TargetSlotIndex == SourceSlotIndex)
	{
		return false;
	}

	CachedViewModel->RequestMoveItem(SourceSlotIndex, TargetSlotIndex);
	return true;
}
