// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/TechTree/DC1TechTreeRowWidget.h"

#include "UI/Button/AnimButton/Tab/DC1TechTreeButton.h"

#include "Kismet/GameplayStatics.h"
#include "Data/DataTable/DC1DataTableTypes.h"

#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"

void UDC1TechTreeRowWidget::InitTier(int32 Level)
{
	if (true == ::IsValid(LevelText))
	{
		LevelText->SetText(FText::AsNumber(Level));
	}

}

UDC1TechTreeButton* UDC1TechTreeRowWidget::AddTechSlot(const FDC1TechRow& Row)
{
	if (false == ::IsValid(SlotContainer) || false == ::IsValid(SlotWidgetClass))
	{
		return nullptr;
	}
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (false == ::IsValid(PC))
	{
		return nullptr;
	}
	UDC1TechTreeButton* SlotWidget = CreateWidget<UDC1TechTreeButton>(PC, SlotWidgetClass);
	if (false == ::IsValid(SlotWidget))
	{
		return nullptr;
	}

	SlotWidget->InitTechButton(Row);
	SlotContainer->AddChild(SlotWidget);

	return SlotWidget;
}