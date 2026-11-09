// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tooltip/DC1TechTooltipWidget.h"

#include "Data/DataTable/DC1DataTableTypes.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDC1TechTooltipWidget::SetTech(const FDC1TechRow& InRow, const FText& InDescription, const TArray<FDC1ItemCost>& InCosts)
{
	if (true == ::IsValid(Text_Name))
	{
		Text_Name->SetText(InRow.DisplayName);
	}

	if (true == ::IsValid(Text_Category))
	{
		Text_Category->SetText(InRow.CategoryName);
	}

	if (true == ::IsValid(Text_Description))
	{
		Text_Description->SetText(InDescription);
	}

	if (true == ::IsValid(Text_TechPoint))
	{
		Text_TechPoint->SetText(FText::AsNumber(InRow.UnlockCost, &FNumberFormattingOptions::DefaultNoGrouping()));
	}

	if (true == ::IsValid(Image_Icon))
	{
		if (false == InRow.Icon.IsNull())
		{
			Image_Icon->SetBrushFromSoftTexture(InRow.Icon);
			Image_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			Image_Icon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	ResetMaterials();
	for (const FDC1ItemCost& Cost : InCosts)
	{
		AddMaterial(Cost.ItemID, Cost.Count);
	}
}
