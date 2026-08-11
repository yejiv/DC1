// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/Inventory/DC1InventorySlotWidget.h"

#include "ViewModel/Inventory/DC1InventoryViewModel.h"
#include "Component/Inventory/DC1InventoryTypes.h"
#include "Components/SizeBox.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h" // BorderSlot (UCanvasPanelSlot) 관련 Definition

void UDC1InventorySlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	//CreateUI();
}

void UDC1InventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SizeBox_Root->SetWidthOverride(50);
	SizeBox_Root->SetHeightOverride(50);
}


void UDC1InventorySlotWidget::SetSlotData(const UDC1InventoryViewModel* ViewModel, int32 SlotIndex)
{
	if (!IsValid(ViewModel))
	{
		ClearSlot();
		return;
	}

	CurrentSlotIndex = SlotIndex;
	CurrentSlot = ViewModel->GetSlot(SlotIndex);

	UpdateSlotUI();
}

void UDC1InventorySlotWidget::ClearSlot()
{
	CurrentSlotIndex = -1;
	CurrentSlot.Clear();

	if (IsValid(ItemIcon))
	{
		ItemIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	if (IsValid(StackCountText))
	{
		StackCountText->SetVisibility(ESlateVisibility::Hidden);
	}

	if (IsValid(SlotBorder))
	{
		SlotBorder->SetBrushColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f));
	}
}

void UDC1InventorySlotWidget::UpdateSlotUI()
{
	if (CurrentSlot.IsEmpty())
	{
		ClearSlot();
		return;
	}

	//const FDC1ItemRow* ItemData = CachedViewModel->GetItemData(CurrentSlot.ItemID);
	//if (!ItemData)
	//{
	//	ClearSlot();
	//	return;
	//}

	//// 레어도 색상 적용
	//UpdateRarityColor(ItemData->ItemRarity);

	//// 아이콘 표시
	//if (IsValid(ItemIcon))
	//{
	//	if (ItemData->ItemIcon.IsPending())
	//	{
	//		ItemIcon->SetBrushFromTexture(nullptr);
	//	}
	//	else
	//	{
	//		ItemIcon->SetBrushFromTexture(ItemData->ItemIcon.Get());
	//	}
	//	ItemIcon->SetVisibility(ESlateVisibility::Visible);
	//}

	//// 스택 수 표시 (1개 이상일 때만)
	//if (IsValid(StackCountText))
	//{
	//	if (CurrentSlot.StackCount > 1)
	//	{
	//		StackCountText->SetText(FText::AsNumber(CurrentSlot.StackCount));
	//		StackCountText->SetVisibility(ESlateVisibility::Visible);
	//	}
	//	else
	//	{
	//		StackCountText->SetVisibility(ESlateVisibility::Hidden);
	//	}
	//}
}

void UDC1InventorySlotWidget::UpdateRarityColor(EDC1ItemRarity Rarity)
{
	if (!IsValid(SlotBorder))
	{
		return;
	}

	FLinearColor RarityColor = DC1InventoryRarityColor::GetRarityColor(Rarity);
	SlotBorder->SetBrushColor(RarityColor);
}
