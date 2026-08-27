// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/Inventory/DC1InventorySlotWidget.h"

#include "UI/Tab/Inventory/DC1InventoryEntryWidget.h"
#include "ViewModel/Inventory/DC1InventoryViewModel.h"
#include "Component/Inventory/DC1InventoryTypes.h"
#include "Data/DataTable/DC1DataTableTypes.h"

#include "Components/SizeBox.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDC1InventorySlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UDC1InventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// EntryWidget 생성 (자식 위젯으로 추가)
	if (IsValid(SizeBox_Root) && !IsValid(EntryWidget))
	{
		EntryWidget = CreateWidget<UDC1InventoryEntryWidget>(this);
		if (IsValid(EntryWidget))
		{
			SizeBox_Root->AddChild(EntryWidget);
		}
	}

	HideHoverImage();
}

void UDC1InventorySlotWidget::InitSlot(int32 SlotIndex, float SlotSize)
{
	CurrentSlotIndex = SlotIndex;

	if (IsValid(SizeBox_Root))
	{
		SizeBox_Root->SetWidthOverride(SlotSize);
		SizeBox_Root->SetHeightOverride(SlotSize);
	}

}

void UDC1InventorySlotWidget::SetSlotData(const UDC1InventoryViewModel* ViewModel)
{
	if (!IsValid(ViewModel))
	{
		ClearSlot();
		return;
	}

	const FDC1InventorySlotInfo& InventorySlot = ViewModel->GetSlot(CurrentSlotIndex);

	if (InventorySlot.IsEmpty())
	{
		ClearSlot();
		return;
	}

	// DataTable에서 아이템 데이터 조회
	const FDC1ItemRow* ItemData = ViewModel->GetItemData(InventorySlot.ItemID);
	if (!ItemData)
	{
		ClearSlot();
		return;
	}

	// 레어도 색상 적용
	UpdateRarityColor(ItemData->ItemRarity);

	// EntryWidget에 데이터 전달
	if (IsValid(EntryWidget))
	{
		EntryWidget->SetEntryData(ViewModel, CurrentSlotIndex);
	}
}

void UDC1InventorySlotWidget::ClearSlot()
{
	// 레어도 색상 초기화
	UpdateRarityColor(EDC1ItemRarity::Common);

	// EntryWidget 초기화
	if (IsValid(EntryWidget))
	{
		EntryWidget->ClearEntry();
	}
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

void UDC1InventorySlotWidget::ShowHoverImage()
{
	if (IsValid(Image_Bracket))
	{
		Image_Bracket->SetVisibility(ESlateVisibility::Visible);

		FDC1AnimPlayInfo PlayInfo;

		// 호버 동안 무한 루프, 언호버 시 원래 값으로 복원
		PlayInfo.PlayMode = EUMGSequencePlayMode::Forward;
		PlayInfo.bRestoreState = true;
		PlayInfo.NumLoops = 0;

		PlayAnim(HoverAnim, PlayInfo);
	}
}

void UDC1InventorySlotWidget::HideHoverImage()
{
	if (IsValid(Image_Bracket))
	{
		Image_Bracket->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UDC1InventorySlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	ShowHoverImage();
}

void UDC1InventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	HideHoverImage();
}
