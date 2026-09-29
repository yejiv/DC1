#include "UI/Tab/Inventory/Slot/DC1InventoryEntryWidget.h"

#include "UI/Tab/Inventory/Slot/DC1InventorySlotWidget.h"
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

void UDC1InventoryEntryWidget::SetEntryData(const UDC1InventoryViewModel* ViewModel, int32 SlotIndex, EDC1InventorySource InSlotSource /* = EDC1InventorySource::Player */)
{
	if (!IsValid(ViewModel))
	{
		ClearEntry();
		return;
	}

	CurrentSlotIndex = SlotIndex;
	const FDC1InventorySlotInfo& InventorySlotInfo = ViewModel->GetSlotBySource(InSlotSource, SlotIndex);

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
		// 소프트 레퍼런스 아이콘: 미로드 상태면 비동기 로드 후 자동으로 브러시 설정
		// (IsPending일 때 nullptr을 넣으면 흰색 사각형으로 표시됨)
		if (true == ItemData->ItemIcon.IsNull())
		{
			// 아이콘이 지정되지 않은 아이템
			Image_Icon->SetBrushFromTexture(nullptr);
		}
		else
		{
			Image_Icon->SetBrushFromSoftTexture(ItemData->ItemIcon);
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