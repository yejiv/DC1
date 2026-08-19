// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/TechTree/DC1TechTreeRowWidget.h"

#include "UI/Button/AnimButton/Tab/DC1TechTreeButton.h"

#include "Kismet/GameplayStatics.h"
#include "Data/DataTable/DC1DataTableTypes.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"

void UDC1TechTreeRowWidget::InitTier(int32 Level)
{
	if (true == ::IsValid(LevelText))
	{
		LevelText->SetText(FText::AsNumber(Level));
	}

	RowLevel = Level;
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

void UDC1TechTreeRowWidget::SetState(int32 NewLevel)
{
	bool IsActivated = RowLevel <= NewLevel;

	bool IsActivatedTopLine = (IsActivated && RowLevel > 1);
	
	bool IsActivatedBottomLine = (IsActivated&& NewLevel > RowLevel);

	
	if (true == ::IsValid(Level_HoverImage))
	{
		float HoverOpacity = IsActivated ? 0.f : InactiveHoverOpacity;
		Level_HoverImage->SetOpacity(HoverOpacity);
	}

	if (true == ::IsValid(Level_Icon))
	{
		FLinearColor IconColor = IsActivated ? ActiveIconColor : InactiveIconColor;
		Level_Icon->SetColorAndOpacity(IconColor);
	}

	if (true == ::IsValid(Level_Background))
	{
		float LevelBackgroundOpacity = IsActivated ? 1.f : 0.f;
		Level_Background->SetOpacity(LevelBackgroundOpacity);
	}

	if (true == ::IsValid(Row_Background))
	{
		FLinearColor RowBackgroundColor = IsActivated ? ActiveBackgroundColor : InactiveBackgroundColor;
		Row_Background->SetColorAndOpacity(RowBackgroundColor);
	}

	if (true == ::IsValid(Level_TopLine))
	{
		float TopLineOpacity = IsActivatedTopLine ? 1.f : 0.f;
		Level_TopLine->SetOpacity(TopLineOpacity);
	}

	if (true == ::IsValid(Level_BottomLine))
	{
		float BottomLineOpacity = IsActivatedBottomLine ? 1.f : 0.f;
		Level_BottomLine->SetOpacity(BottomLineOpacity);
	}
}