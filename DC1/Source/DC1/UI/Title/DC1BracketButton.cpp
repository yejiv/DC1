// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Title/DC1BracketButton.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"

void UDC1BracketButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (true == ::IsValid(TextLabel))
	{
		TextLabel->SetText(Button_LabelText);
	}

	if (true == ::IsValid(BracketButton))
	{
		BracketButton->OnClicked.AddDynamic(this, &ThisClass::HandleClicked);
		BracketButton->OnHovered.AddDynamic(this, &ThisClass::HandleHovered);
		BracketButton->OnUnhovered.AddDynamic(this, &ThisClass::HandleUnhovered);
	}
	if (true == ::IsValid(BracketImage))
	{
		BracketImage->SetOpacity(0.f);
	}
}

void UDC1BracketButton::HandleClicked()
{
	OnClickedDelegate.Broadcast();
}

void UDC1BracketButton::HandleHovered()
{
	if (true == ::IsValid(BracketImage))
	{
		BracketImage->SetOpacity(1.f);
	}

	if (true == ::IsValid(HoverAnim))
	{
		PlayAnimation(HoverAnim, 0.f, 0);
	}
}

void UDC1BracketButton::HandleUnhovered()
{
	if (true == ::IsValid(BracketImage))
	{
		BracketImage->SetOpacity(0.f);
	}

	if (true == ::IsValid(HoverAnim))
	{
		StopAnimation(HoverAnim);
	}
}
