// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Craft/DC1CraftMaterialEntryWidget.h"

#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1ItemHelper.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDC1CraftMaterialEntryWidget::SetMaterial(FName InItemID, int32 InOwnedCount, int32 InRequiredCount)
{
	const FDC1ItemRow* ItemRow = DC1ItemHelper::FindItemRow(InItemID);

	if (true == ::IsValid(Image_Icon))
	{
		if (nullptr != ItemRow && false == ItemRow->ItemIcon.IsNull())
		{
			Image_Icon->SetBrushFromSoftTexture(ItemRow->ItemIcon);
			Image_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			Image_Icon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (true == ::IsValid(Text_Name))
	{
		Text_Name->SetText((nullptr != ItemRow) ? ItemRow->ItemName : FText::FromName(InItemID));
	}

	if (true == ::IsValid(Text_Count))
	{
		const FNumberFormattingOptions& NoGrouping = FNumberFormattingOptions::DefaultNoGrouping();

		Text_Count->SetText(FText::Format(NSLOCTEXT("Craft", "MaterialCount", "{0} / {1}"),
			FText::AsNumber(InOwnedCount, &NoGrouping), FText::AsNumber(InRequiredCount, &NoGrouping)));
	}

	if (true == ::IsValid(InactiveImage))
	{
		InactiveImage->SetOpacity(InOwnedCount >= InRequiredCount ? 0.f : InactiveImageOpacity);
	}
}