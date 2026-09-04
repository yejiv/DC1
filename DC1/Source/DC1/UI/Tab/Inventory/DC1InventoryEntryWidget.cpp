#include "UI/Tab/Inventory/DC1InventoryEntryWidget.h"

#include "UI/Tab/Inventory/DC1InventorySlotWidget.h"
#include "ViewModel/Inventory/DC1InventoryViewModel.h"
#include "Component/Inventory/DC1InventoryTypes.h"
#include "Data/DataTable/DC1DataTableTypes.h"

#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

UDC1InventoryEntryWidget::UDC1InventoryEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetVisibility(ESlateVisibility::Visible);
}

void UDC1InventoryEntryWidget::SetEntryData(const UDC1InventoryViewModel* ViewModel, int32 SlotIndex, bool bIsChestSlot /* = false */)
{
	if (!IsValid(ViewModel))
	{
		ClearEntry();
		return;
	}

	CurrentSlotIndex = SlotIndex;
	const FDC1InventorySlotInfo& InventorySlotInfo = bIsChestSlot ? ViewModel->GetChestSlot(SlotIndex) : ViewModel->GetSlot(SlotIndex);

	if (InventorySlotInfo.IsEmpty())
	{
		ClearEntry();
		return;
	}

	CurrentItemID = InventorySlotInfo.ItemID;
	CurrentStackCount = InventorySlotInfo.StackCount;

	// DataTable에서 아이템 데이터 조회
	const FDC1ItemRow* ItemData = ViewModel->GetItemData(InventorySlotInfo.ItemID);
	if (!ItemData)
	{
		ClearEntry();
		return;
	}

	if (IsValid(Image_Icon))
	{
		if (ItemData->ItemIcon.IsPending())
		{
			Image_Icon->SetBrushFromTexture(nullptr);
		}
		else
		{
			Image_Icon->SetBrushFromTexture(ItemData->ItemIcon.Get());
		}
		Image_Icon->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
		Image_Icon->SetVisibility(ESlateVisibility::Visible);
	}

	if (IsValid(Text_Count))
	{
		if (InventorySlotInfo.StackCount >= 1)
		{
			Text_Count->SetText(FText::AsNumber(InventorySlotInfo.StackCount));
			Text_Count->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Text_Count->SetText(FText::AsNumber(0));
			Text_Count->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// 무게 표시
	if (IsValid(Text_Weight))
	{
		CurrentItemWeight = ItemData->Weight;
		float TotalWeight = CurrentItemWeight * InventorySlotInfo.StackCount;
		Text_Weight->SetText(FText::AsNumber(TotalWeight));
		Text_Weight->SetVisibility(ESlateVisibility::Visible);
	}
}

void UDC1InventoryEntryWidget::ClearEntry()
{
	CurrentSlotIndex = -1;
	CurrentItemID = NAME_None;
	CurrentStackCount = 0;
	CurrentItemWeight = 0.0f;

	if (IsValid(Image_Icon))
	{
		Image_Icon->SetBrushFromTexture(nullptr);
		Image_Icon->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.0f));
		Image_Icon->SetVisibility(ESlateVisibility::Hidden);
	}

	if (IsValid(Text_Count))
	{
		Text_Count->SetText(FText::AsNumber(0));
		Text_Count->SetVisibility(ESlateVisibility::Hidden);
	}

	if (IsValid(Text_Weight))
	{
		Text_Weight->SetText(FText::AsNumber(0));
		Text_Weight->SetVisibility(ESlateVisibility::Hidden);
	}
}


void UDC1InventoryEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDC1InventoryEntryWidget::RefreshItemCount(int32 NewItemCount)
{
	if (IsValid(Text_Count))
	{
		Text_Count->SetText(FText::AsNumber(NewItemCount));
	}
}

void UDC1InventoryEntryWidget::SetWeightVisibility(bool bVisible)
{
	if (IsValid(Text_Weight))
	{
		Text_Weight->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}