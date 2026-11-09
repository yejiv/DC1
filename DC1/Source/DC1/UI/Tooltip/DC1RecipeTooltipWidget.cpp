// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tooltip/DC1RecipeTooltipWidget.h"

#include "UI/Tooltip/DC1ItemHeaderWidget.h"

#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1ItemHelper.h"

#include "Components/TextBlock.h"

void UDC1RecipeTooltipWidget::SetRecipe(const FDC1ItemRecipeRow& InRecipe)
{
	if (true == ::IsValid(ItemHeader))
	{
		ItemHeader->SetItem(InRecipe.ProductItemID);
	}

	const FDC1ItemRow* ProductRow = DC1ItemHelper::FindItemRow(InRecipe.ProductItemID);

	if (true == ::IsValid(Text_Description))
	{
		Text_Description->SetText((nullptr != ProductRow) ? ProductRow->Description : FText::GetEmpty());
	}

	ResetMaterials();
	for (const FDC1RecipeItem& RequiredItem : InRecipe.RequiredItems)
	{
		AddMaterial(RequiredItem.ItemID, RequiredItem.Count);
	}
}
