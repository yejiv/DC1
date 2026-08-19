// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/TechTree/DC1TechTreeRowWidget.h"

#include "UI/Button/AnimButton/Slot/DC1TechTreeButton.h"

#include "Data/DataTable/DC1DataTableTypes.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"

void UDC1TechTreeRowWidget::InitTier(int32 InLevel)
{
	if (true == ::IsValid(LevelText))
	{
		LevelText->SetText(FText::AsNumber(InLevel));
	}

	RowLevel = InLevel;
}

UDC1TechTreeButton* UDC1TechTreeRowWidget::AddTechSlot(const FDC1TechRow& InRow)
{
	if (false == ::IsValid(SlotContainer) || false == ::IsValid(SlotWidgetClass))
	{
		return nullptr;
	}

	UDC1TechTreeButton* SlotWidget = CreateWidget<UDC1TechTreeButton>(this, SlotWidgetClass);
	if (false == ::IsValid(SlotWidget))
	{
		return nullptr;
	}

	SlotWidget->InitTechButton(InRow);
	SlotContainer->AddChild(SlotWidget);

	return SlotWidget;
}

void UDC1TechTreeRowWidget::SetState(int32 InNewLevel)
{
	bool IsActivated = RowLevel <= InNewLevel;

	bool IsActivatedTopLine = (IsActivated && RowLevel > 1);
	
	bool IsActivatedBottomLine = (IsActivated&& InNewLevel > RowLevel);

	ApplyState(IsActivated, IsActivatedTopLine, IsActivatedBottomLine);
}

void UDC1TechTreeRowWidget::ApplyState(bool InIsActivated, bool InIsActivatedTopLine, bool InIsActivatedBottomLine)
{
	if (true == ::IsValid(Level_HoverImage))
	{
		float HoverOpacity = InIsActivated ? 0.f : InactiveHoverOpacity;
		Level_HoverImage->SetOpacity(HoverOpacity);
	}

	if (true == ::IsValid(Level_Icon))
	{
		FLinearColor IconColor = InIsActivated ? ActiveIconColor : InactiveIconColor;
		Level_Icon->SetColorAndOpacity(IconColor);
	}

	if (true == ::IsValid(Level_Background))
	{
		float LevelBackgroundOpacity = InIsActivated ? 1.f : 0.f;
		Level_Background->SetOpacity(LevelBackgroundOpacity);
	}

	if (true == ::IsValid(Row_Background))
	{
		FLinearColor RowBackgroundColor = InIsActivated ? ActiveBackgroundColor : InactiveBackgroundColor;
		Row_Background->SetColorAndOpacity(RowBackgroundColor);
	}

	if (true == ::IsValid(Level_TopLine))
	{
		float TopLineOpacity = InIsActivatedTopLine ? 1.f : 0.f;
		Level_TopLine->SetOpacity(TopLineOpacity);
	}

	if (true == ::IsValid(Level_BottomLine))
	{
		float BottomLineOpacity = InIsActivatedBottomLine ? 1.f : 0.f;
		Level_BottomLine->SetOpacity(BottomLineOpacity);
	}
}