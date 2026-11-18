// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tooltip/DC1InventoryTooltipWidget.h"

#include "UI/Tooltip/DC1ItemHeaderWidget.h"

#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1ItemHelper.h"

#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"

void UDC1InventoryTooltipWidget::SetInventoryItem(FName InItemID, int32 InStackCount)
{
	if (true == ::IsValid(ItemHeader))
	{
		ItemHeader->SetItem(InItemID);
	}

	const FDC1ItemRow*ItemRow = DC1ItemHelper::FindItemRow(InItemID);

	if (true == ::IsValid(Text_Description))
	{
		Text_Description->SetText((nullptr != ItemRow) ? ItemRow->Description : FText::GetEmpty());
	}

	if (true == ::IsValid(WeightBox))
	{
		const bool HasWeight = (nullptr != ItemRow) && (false == DC1ItemHelper::IsValuableItem(InItemID));
		WeightBox->SetVisibility(HasWeight ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

		if (true == HasWeight && true == ::IsValid(Text_Weight))
		{
			FNumberFormattingOptions WeightFormat;
			WeightFormat.SetUseGrouping(false).SetMinimumFractionalDigits(0).SetMaximumFractionalDigits(1);
			Text_Weight->SetText(FText::AsNumber(ItemRow->Weight * InStackCount, &WeightFormat));
		}
	}

	if (true == ::IsValid(CountBox))
	{
		FNumberFormattingOptions CountFormat;
		CountFormat.SetUseGrouping(false).SetMaximumFractionalDigits(0);
		Text_Count->SetText(FText::AsNumber(InStackCount, &CountFormat));
	}
}