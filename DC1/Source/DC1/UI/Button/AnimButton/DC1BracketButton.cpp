// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Button/AnimButton/DC1BracketButton.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Animation/WidgetAnimation.h"

void UDC1BracketButton::NativeConstruct()
{
	Super::NativeConstruct();

	if(true == ::IsValid(ButtonSizeBox))
	{ 
		ButtonSizeBox->SetWidthOverride(ButtonSizeX);
		ButtonSizeBox->SetHeightOverride(ButtonSizeY);
	}

	if (true == ::IsValid(TextLabel))
	{
		TextLabel->SetText(Button_LabelText);
		TextLabel->SetFontSize(FontSize);
	}

	if (true == ::IsValid(BracketImage))
	{
		BracketImage->SetOpacity(0.f);
	}


	if (true == ButtonBrushOverride)
	{
		FButtonStyle Style = UserButton->GetStyle();

		Style.Normal = NormalBrush;
		Style.Hovered = HoveredBrush;
		Style.Pressed = PressedBrush;

		UserButton->SetStyle(Style);
	}
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
