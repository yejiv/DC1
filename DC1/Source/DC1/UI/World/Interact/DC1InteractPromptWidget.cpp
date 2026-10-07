// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/Interact/DC1InteractPromptWidget.h"


#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UI/World/Interact/DC1InteractPromptRowWidget.h"

void UDC1InteractPromptWidget::SetOptions(const TArray<FDC1InteractOption>& InOptions)
{
	if (false == ::IsValid(PromptVerticalBox))
	{
		return;
	}

	PromptVerticalBox->ClearChildren();
	RowWidgetByKey.Empty();

	bool HasChild = false;

	for (const FDC1InteractOption& Option : InOptions)
	{
		UDC1InteractPromptRowWidget* RowWidget = CreateWidget<UDC1InteractPromptRowWidget>(this, RowWidgetClass);
		if (false == ::IsValid(RowWidget))
		{
			continue;
		}

		RowWidget->SetOption(Option);
		RowWidget->SetFillRatio(0.f);
		
		int32 RowIndex = PromptVerticalBox->GetChildrenCount();
		UVerticalBoxSlot* RowSlot = PromptVerticalBox->AddChildToVerticalBox(RowWidget);
		if (true == ::IsValid(RowSlot) && RowIndex > 0)
		{
			RowSlot->SetPadding(FMargin(0.f, RowSpacing, 0.f, 0.f));
		}

		RowWidgetByKey.Add(Option.Key, RowWidget);

	}
}

void UDC1InteractPromptWidget::SetKeyHighlighted(EDC1InteractKey InKey, bool IsInHighlighted)
{
	SetHoldRatio(InKey, IsInHighlighted ? 1.f : 0.f);
}

void UDC1InteractPromptWidget::SetHoldRatio(EDC1InteractKey InKey, float InRatio)
{
	UDC1InteractPromptRowWidget* RowWidget = RowWidgetByKey.FindRef(InKey);
	if (false == ::IsValid(RowWidget))
	{
		return;
	}

	RowWidget->SetFillRatio(InRatio);
}
