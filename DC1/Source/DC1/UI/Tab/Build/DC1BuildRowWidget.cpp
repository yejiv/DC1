// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/Build/DC1BuildRowWidget.h"

#include "UI/Button/AnimButton/Slot/DC1BuildButton.h"

#include "Data/DataTable/DC1DataTableTypes.h"

#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"

void UDC1BuildRowWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	
}

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

UDC1BuildButton* UDC1BuildRowWidget::AddBuildSlot(const FDC1BuildRow& InBuildRow, const FDC1TechRow& InTechRow)
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
	SlotWidget->InitBuildButton(InBuildRow, InTechRow);

	SlotContainer->AddChild(SlotWidget);
	Slots.FindOrAdd(InBuildRow.BuildTag, SlotWidget);

	SlotWidget->OnClickedDelegate.AddWeakLambda(this, []()
		{
			UE_LOG(LogTemp, Warning, TEXT("Need Bind Build Function"));
		});


	return SlotWidget;
}

bool UDC1BuildRowWidget::VisibleSlot(const FGameplayTag& InSlotTag)
{
	TObjectPtr<UDC1BuildButton>* Found = Slots.Find(InSlotTag);
	if (nullptr == Found)
	{
		return false;
	}

	(*Found)->SetVisibility(ESlateVisibility::Visible);
	
	if (ESlateVisibility::Visible != GetVisibility())
	{
		this->SetVisibility(ESlateVisibility::Visible);
	}

	return true;
}
