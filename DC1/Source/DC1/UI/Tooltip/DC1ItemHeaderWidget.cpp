// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tooltip/DC1ItemHeaderWidget.h"

#include "UI/Tooltip/DC1TooltipWidget.h"

#include "Component/Inventory/DC1InventoryTypes.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1ItemHelper.h"

#include "Components/PanelWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDC1ItemHeaderWidget::SetItem(FName InItemID)
{
	const FDC1ItemRow* ItemRow = DC1ItemHelper::FindItemRow(InItemID);

	if (true == ::IsValid(Text_Name))
	{
		Text_Name->SetText((nullptr != ItemRow) ? ItemRow->ItemName : FText::FromName(InItemID));
	}

	if (true == ::IsValid(Text_ItemType))
	{
		Text_ItemType->SetText((nullptr != ItemRow) ? UDC1TooltipWidget::FindTagName(ItemTypeNamesByTag, ItemRow->CategoryTag) : FText::GetEmpty());
	}

	if (true == ::IsValid(Text_ItemRarity))
	{
		Text_ItemRarity->SetText((nullptr != ItemRow) ? DC1InventoryRarityText::GetRarityText(ItemRow->ItemRarity) : FText::GetEmpty());
	}

	if (true == ::IsValid(Image_HeaderBack))
	{
		FLinearColor BackColor = DC1InventoryRarityColor::GetRarityColor((nullptr != ItemRow) ? ItemRow->ItemRarity : EDC1ItemRarity::Common);
		BackColor.A = 1.f;
		Image_HeaderBack->SetColorAndOpacity(BackColor);
	}

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

	const FDC1EquipItemRow* EquipRow = DC1ItemHelper::FindEquipItemRow(InItemID);
	SetStatLine(AttackBox, Text_Attack, (nullptr != EquipRow) ? EquipRow->AttackPower : 0);
	SetStatLine(DefenseBox, Text_Defense, (nullptr != EquipRow) ? EquipRow->DefensePower : 0);
	SetStatLine(HealthBox, Text_Health, (nullptr != EquipRow) ? EquipRow->Health : 0);

	const FDC1FoodItemRow* FoodRow = DC1ItemHelper::FindFoodItemRow(InItemID);
	SetStatLine(SatietyBox, Text_Satiety, (nullptr != FoodRow) ? FMath::RoundToInt(FoodRow->SatietyAmount) : 0);
}

void UDC1ItemHeaderWidget::SetStatLine(UPanelWidget* InLineBox, UTextBlock* InValueText, int32 InValue)
{
	if (false == ::IsValid(InLineBox))
	{
		return;
	}

	const bool HasValue = InValue > 0;
	InLineBox->SetVisibility(HasValue ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	if (true == HasValue && true == ::IsValid(InValueText))
	{
		InValueText->SetText(FText::AsNumber(InValue, &FNumberFormattingOptions::DefaultNoGrouping()));
	}
}
