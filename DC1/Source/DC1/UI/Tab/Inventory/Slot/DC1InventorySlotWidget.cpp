// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/Inventory/Slot/DC1InventorySlotWidget.h"

#include "UI/Tab/Inventory/Slot/DC1InventoryEntryWidget.h"
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
	if (!IsValid(EntryWidget) && IsValid(SizeBox_Root))
	{
		EntryWidget = CreateWidget<UDC1InventoryEntryWidget>(this);
		if (IsValid(EntryWidget))
		{
			SizeBox_Root->AddChild(EntryWidget);
		}
	}

	HideHoverImage();
}

void UDC1InventorySlotWidget::InitSlot(int32 SlotIndex, float SlotSize, EDC1InventorySource InSlotSource /* = EDC1InventorySource::Player */)
{
	CurrentSlotIndex = SlotIndex;
	SlotSource = InSlotSource;

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

	const FDC1InventorySlotInfo& InventorySlot = ViewModel->GetSlotBySource(SlotSource, CurrentSlotIndex);

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
		EntryWidget->SetEntryData(ViewModel, CurrentSlotIndex, SlotSource);
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
		PlayInfo.RestoreState = true;
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

void UDC1InventorySlotWidget::SetWeightVisibility(bool bVisible)
{
	if (IsValid(EntryWidget))
	{
		EntryWidget->SetWeightVisibility(bVisible);
	}
}

void UDC1InventorySlotWidget::OverrideDisplayCount(int32 NewCount)
{
	if (IsValid(EntryWidget))
	{
		EntryWidget->RefreshItemCount(NewCount);
	}
}

void UDC1InventorySlotWidget::InitializeDragCopy()
{
	// 드래그 복제본용 스타일 설정
	// HitTestInvisible: 화면에는 보이지만 마우스 입력(클릭, Hover)은 무시하고 아래 위젯으로 통과시킴
	// (마우스를 항상 따라다니므로 Visible로 두면 자기 자신이 클릭/Hover를 가로채 드롭 및 Hover 로직이 깨짐)
	SetVisibility(ESlateVisibility::HitTestInvisible);

	// SlotBorder 스타일 설정 (기본 Common 레어도 색상)
	if (IsValid(SlotBorder))
	{
		UpdateRarityColor(EDC1ItemRarity::Common);
		SlotBorder->SetPadding(FMargin(2.0f));
	}
}
