// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/Build/DC1BuildRowWidget.h"

#include "UI/Button/AnimButton/Slot/DC1BuildButton.h"

#include "Data/DataTable/DC1DataTableTypes.h"

#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"

#include "ViewModel/Build/DC1BuildViewModel.h"


void UDC1BuildRowWidget::InitRow(const FGameplayTag& InRowTag, const FText& InRowName)
{
	if (true == InRowTag.IsValid())
	{
		RowTag = InRowTag;
	}

	if (true == ::IsValid(RowNameText) && false == InRowName.IsEmpty())
	{
		RowNameText->SetText(InRowName);
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

UDC1BuildButton* UDC1BuildRowWidget::AddBuildSlot(const FDC1BuildRow& InBuildRow, UDC1BuildViewModel* InViewModel)
{
	if (false == ::IsValid(SlotContainer) || false == ::IsValid(SlotWidgetClass))
	{
		return nullptr;
	}

	UDC1BuildButton* SlotWidget = CreateWidget<UDC1BuildButton>(this, SlotWidgetClass);
	if (false == ::IsValid(SlotWidget))
	{
		return nullptr;
	}

	// Init Slot Button - Setting Tag, Cost, Image, Visible
	SlotWidget->InitBuildButton(InBuildRow);

	SlotContainer->AddChild(SlotWidget);
	Slots.FindOrAdd(InBuildRow.BuildTag, SlotWidget);

	const FGameplayTag BuildTag = InBuildRow.BuildTag;

	SlotWidget->OnClickedDelegate.AddWeakLambda(this, [this, BuildTag, InViewModel]()
		{
			if (false == ::IsValid(InViewModel) || false == InViewModel->RequestStartPlacement(BuildTag))
			{
				return;
			}

			PopScreen();
		});

	return SlotWidget;
}

bool UDC1BuildRowWidget::VisibleSlot(const FGameplayTag& InSlotTag)
{
	UDC1BuildButton* Found = Slots.FindRef(InSlotTag);
	if (false == ::IsValid(Found))
	{
		return false;
	}

	Found->SetVisibility(ESlateVisibility::Visible);
	
	if (ESlateVisibility::Visible != GetVisibility())
	{
		this->SetVisibility(ESlateVisibility::Visible);
	}

	return true;
}
