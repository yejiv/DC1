// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tooltip/DC1BuildTooltipWidget.h"

#include "Data/DataTable/DC1DataTableTypes.h"

#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UDC1BuildTooltipWidget::SetBuild(const FDC1BuildRow& InRow)
{
	if (true == ::IsValid(Text_Name))
	{
		Text_Name->SetText(InRow.DisplayName);
	}

	if (true == ::IsValid(Text_Description))
	{
		Text_Description->SetText(InRow.Description);
	}

	if (true == ::IsValid(WorkTypeBox))
	{
		const bool HasWorkType = InRow.WorkTypeTag.IsValid();
		WorkTypeBox->SetVisibility(HasWorkType ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

		if (true == HasWorkType && true == ::IsValid(Image_WorkType))
		{
			TSoftObjectPtr<UTexture2D> IconSoftPtr = WorkTypeIconsByTag.FindRef(InRow.WorkTypeTag);
			if (false == IconSoftPtr.IsNull())
			{
				Image_WorkType->SetBrushFromSoftTexture(IconSoftPtr);
			}
		}
	}

	if (true == ::IsValid(Text_RequiredWork))
	{
		FNumberFormattingOptions WorkFormat;
		WorkFormat.SetUseGrouping(false).SetMaximumFractionalDigits(0);
		Text_RequiredWork->SetText(FText::AsNumber(InRow.RequiredWork, &WorkFormat));
	}

	ResetMaterials();
	bool HasAllMaterials = true;
	for (const FDC1ItemCost& Cost : InRow.BuildCosts)
	{
		HasAllMaterials &= AddMaterial(Cost.ItemID, Cost.Count);
	}

	if (true == ::IsValid(InactiveBox))
	{
		InactiveBox->SetVisibility(HasAllMaterials ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
}
