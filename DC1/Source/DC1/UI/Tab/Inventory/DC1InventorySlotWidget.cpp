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

	SizeBox_Root->SetWidthOverride(100);
	SizeBox_Root->SetHeightOverride(100);
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

	if (CurrentSlot.IsEmpty())
	{
		ClearSlot();
		return;
	}

	// DataTable에서 아이템 데이터 조회
	const FDC1ItemRow* ItemData = ViewModel->GetItemData(CurrentSlot.ItemID);
	if (!ItemData)
	{
		ClearSlot();
		return;
	}

	// 레어도 색상 적용
	UpdateRarityColor(ItemData->ItemRarity);

	// 아이콘 표시
	if (IsValid(ItemIcon))
	{
		if (ItemData->ItemIcon.IsPending())
		{
			ItemIcon->SetBrushFromTexture(nullptr);
		}
		else
		{
			ItemIcon->SetBrushFromTexture(ItemData->ItemIcon.Get());
		}
		ItemIcon->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
		ItemIcon->SetVisibility(ESlateVisibility::Visible);
	}

	// 스택 수 표시 (1개 이상일 때만)
	if (IsValid(StackCountText))
	{
		if (CurrentSlot.StackCount >= 1)
		{
			StackCountText->SetText(FText::AsNumber(CurrentSlot.StackCount));
			StackCountText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			StackCountText->SetText(FText::AsNumber(0));
			StackCountText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UDC1InventorySlotWidget::ClearSlot()
{
	CurrentSlotIndex = -1;
	CurrentSlot.Clear();

	if (IsValid(ItemIcon))
	{
		ItemIcon->SetBrushFromTexture(nullptr);
		ItemIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	if (IsValid(StackCountText))
	{
		StackCountText->SetText(FText::AsNumber(0));
		StackCountText->SetVisibility(ESlateVisibility::Hidden);
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
	SlotBorder->Background.TintColor = RarityColor;
}
