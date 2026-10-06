// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/Interact/DC1InteractPromptRowWidget.h"

#include "Interface/DC1InteractableInterface.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UDC1InteractPromptRowWidget::SetOption(const FDC1InteractOption& InOption)
{
	if (false == ::IsValid(KeyText) || false == ::IsValid(DisplayText))
	{
		return;
	}

	KeyText->SetText(UEnum::GetDisplayValueAsText(InOption.Key));
	DisplayText->SetText(InOption.DisplayText);
}

void UDC1InteractPromptRowWidget::SetFillRatio(float InRatio)
{
	if (false == ::IsValid(FillBar))
	{
		return;
	}

	FillBar->SetPercent(InRatio);
}
