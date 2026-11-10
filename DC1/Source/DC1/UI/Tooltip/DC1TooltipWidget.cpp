// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tooltip/DC1TooltipWidget.h"

#include "UI/Craft/DC1CraftMaterialEntryWidget.h"

#include "Character/Player/DC1PlayerState.h"

#include "Define/DC1CraftHelper.h"

#include "Components/DynamicEntryBox.h"

FText UDC1TooltipWidget::FindTagName(const TMap<FGameplayTag, FText>& InTagNames, const FGameplayTag& InTag)
{
	FGameplayTag CurrentTag = InTag;
	while (true == CurrentTag.IsValid())
	{
		if (const FText* TagName = InTagNames.Find(CurrentTag))
		{
			return *TagName;
		}

		CurrentTag = CurrentTag.RequestDirectParent();
	}

	return FText::GetEmpty();
}

void UDC1TooltipWidget::ResetMaterials()
{
	if (true == ::IsValid(MaterialListBox))
	{
		MaterialListBox->Reset();
	}
}

bool UDC1TooltipWidget::AddMaterial(FName InItemID, int32 InRequiredCount)
{
	const int32 OwnedCount = DC1CraftHelper::GetCraftItemCount(GetOwningPlayerState<ADC1PlayerState>(), InItemID);

	if (true == ::IsValid(MaterialListBox))
	{
		UDC1CraftMaterialEntryWidget* Entry = MaterialListBox->CreateEntry<UDC1CraftMaterialEntryWidget>();
		if (true == ::IsValid(Entry))
		{
			Entry->SetMaterial(InItemID, OwnedCount, InRequiredCount);
		}
	}

	return OwnedCount >= InRequiredCount;
}
