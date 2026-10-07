// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/Interact/DC1InteractPromptRowWidget.h"

#include "Interface/DC1InteractableInterface.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"

void UDC1InteractPromptRowWidget::SetOption(const FDC1InteractOption& InOption)
{
	if (false == ::IsValid(KeyImage) || false == ::IsValid(DisplayText))
	{
		return;
	}

	UTexture2D* Icon = KeyIcons.FindRef(InOption.Key);
	if (true == ::IsValid(Icon))
	{
		KeyImage->SetBrushFromTexture(Icon);
	}

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
