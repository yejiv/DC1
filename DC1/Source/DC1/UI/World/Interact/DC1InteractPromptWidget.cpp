// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/Interact/DC1InteractPromptWidget.h"


#include "Components/VerticalBox.h"
#include "UI/World/Interact/DC1InteractPromptRowWidget.h"

void UDC1InteractPromptWidget::SetOptions(const TArray<FDC1InteractOption>& InOptions)
{
	if (false == ::IsValid(PromptVerticalBox))
	{
		return;
	}

	PromptVerticalBox->ClearChildren();
	RowWidgetByKey.Empty();

	for (const FDC1InteractOption& Option : InOptions)
	{
		UDC1InteractPromptRowWidget* RowWidget = CreateWidget<UDC1InteractPromptRowWidget>(this, RowWidgetClass);
		if (false == ::IsValid(RowWidget))
		{
			continue;
		}

		RowWidget->SetOption(Option);
		RowWidget->SetFillRatio(0.f);

		PromptVerticalBox->AddChild(RowWidget);
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
