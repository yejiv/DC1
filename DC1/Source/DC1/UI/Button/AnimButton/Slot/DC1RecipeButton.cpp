// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Button/AnimButton/Slot/DC1RecipeButton.h"

#include "Component/Inventory/DC1InventoryTypes.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1ItemHelper.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDC1RecipeButton::NativeConstruct()
{
	Super::NativeConstruct();

	ApplySelectVisual(IsSelected);
}

void UDC1RecipeButton::HandleHovered()
{
	Super::HandleHovered();

	ApplySelectVisual(true);
}

void UDC1RecipeButton::HandleUnhovered()
{
	Super::HandleUnhovered();

	ApplySelectVisual(IsSelected);
}

void UDC1RecipeButton::HandleClicked()
{
	if (false == CanCraft)
	{
		return;
	}

	Super::HandleClicked();

	OnRecipeClicked.Broadcast(RecipeID);
}

void UDC1RecipeButton::SetRecipe(const FDC1ItemRecipeRow& InRecipe, bool InCanCraft)
{
	RecipeID = InRecipe.RecipeID;
	CanCraft = InCanCraft;

	ApplyState();

	const FDC1ItemRow* ProductRow = DC1ItemHelper::FindItemRow(InRecipe.ProductItemID);

	if (true == ::IsValid(RecipeIconImage))
	{
		if (nullptr != ProductRow && false == ProductRow->ItemIcon.IsNull())
		{
			RecipeIconImage->SetBrushFromSoftTexture(ProductRow->ItemIcon);
			RecipeIconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			RecipeIconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (true == ::IsValid(IconBackImage) && nullptr != ProductRow)
	{
		FLinearColor BackColor = DC1InventoryRarityColor::GetRarityColor(ProductRow->ItemRarity);
		BackColor.A = 1.f;

		IconBackImage->SetColorAndOpacity(BackColor);
	}

	if (true == ::IsValid(ButtonBottomImage) && nullptr != ProductRow)
	{
		bool IsHidden = EDC1ItemRarity::Common == ProductRow->ItemRarity;
		
		if (true == IsHidden)
		{
			ButtonBottomImage->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			FLinearColor BottomColor = DC1InventoryRarityColor::GetRarityColor(ProductRow->ItemRarity);
			BottomColor.A = ProductRow->ItemRarity == EDC1ItemRarity::Common ? 0.f : 1.f;

			ButtonBottomImage->SetColorAndOpacity(BottomColor);
			ButtonBottomImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}


	if (true == ::IsValid(ProductCountText))
	{
		if (InRecipe.ProductCount > 1)
		{
			ProductCountText->SetText(FText::Format(NSLOCTEXT("Craft", "RecipeProductCount", "×{0}"), FText::AsNumber(InRecipe.ProductCount, &FNumberFormattingOptions::DefaultNoGrouping())));
			ProductCountText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			ProductCountText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UDC1RecipeButton::ApplyState()
{
	if (true == ::IsValid(InactiveImage))
	{
		float ImageOpacity = CanCraft ? 0.f : InactiveImageOpacity;
		InactiveImage->SetOpacity(ImageOpacity);
	}
}

void UDC1RecipeButton::SetSelected(bool InSelected)
{
	IsSelected = InSelected;

	ApplySelectVisual(IsSelected || IsHovered());
}

void UDC1RecipeButton::ApplySelectVisual(bool InShow)
{
	if (true == ::IsValid(BracketImage))
	{
		BracketImage->SetOpacity(InShow ? 1.f : 0.f);
	}

	if (true == ::IsValid(ButtonImage))
	{
		ButtonImage->SetBrush(InShow ? HoverButtonImageBrush : UnHoverButtonImageBrush);
	}
}
