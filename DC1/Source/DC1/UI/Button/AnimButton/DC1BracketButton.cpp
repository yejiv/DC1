// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Button/AnimButton/DC1BracketButton.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Animation/WidgetAnimation.h"

void UDC1BracketButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (true == ::IsValid(TextLabel))
	{
		if (false == Button_LabelText.IsEmpty())
		{
			TextLabel->SetText(Button_LabelText);
		}

		if (0.f < FontSize)
		{
			TextLabel->SetFontSize(FontSize);
		}
	}

	if (true == ::IsValid(ButtonSizeBox))
	{
		if (0.f < ButtonSizeX)
		{
			ButtonSizeBox->SetWidthOverride(ButtonSizeX);
		}

		if (0.f < ButtonSizeY)
		{
			ButtonSizeBox->SetHeightOverride(ButtonSizeY);
		}
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

void UDC1BracketButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (true == ::IsValid(BracketImage))
	{
		BracketImage->SetOpacity(0.f);
	}
}

void UDC1BracketButton::HandleHovered()
{
	if (true == ::IsValid(BracketImage))
	{
		BracketImage->SetOpacity(1.f);
	}

	FDC1AnimPlayInfo PlayInfo;

	// 호버 동안 무한 루프, 언호버 시 원래 값으로 복원
	PlayInfo.PlayMode = EUMGSequencePlayMode::Forward;
	PlayInfo.bRestoreState = true;
	PlayInfo.NumLoops = 0;

	PlayAnim(HoverAnim, PlayInfo);
}

void UDC1BracketButton::HandleUnhovered()
{
	if (true == ::IsValid(BracketImage))
	{
		BracketImage->SetOpacity(0.f);
	}

	StopAnim(HoverAnim);
}
