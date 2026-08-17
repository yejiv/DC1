// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Button/AnimButton/Tab/DC1TabHeaderButton.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"

void UDC1TabHeaderButton::NativeConstruct()
{
	Super::NativeConstruct();

	SetActive(false);

	if (true == ::IsValid(HoverImage))
	{
		HoverImage->SetOpacity(0.f);
	}
}

void UDC1TabHeaderButton::HandleHovered()
{
	Super::HandleHovered();

	if (true == ::IsValid(HoverImage))
	{
		HoverImage->SetOpacity(HoverOpacity);
	}
}

void UDC1TabHeaderButton::HandleUnhovered()
{
	Super::HandleUnhovered();

	if (true == ::IsValid(HoverImage))
	{
		HoverImage->SetOpacity(0.f);
	}
}

void UDC1TabHeaderButton::SetActive(bool IsActive)
{
	if (false == ::IsValid(UserButton))
	{
		return;
	}

	if (true == ::IsValid(ClickedAnim))
	{
		if (true == IsActive)
		{
			PlayAnimation(ClickedAnim, 0.f, 1, EUMGSequencePlayMode::Forward);
		}
		else
		{
			PlayAnimation(ClickedAnim, 0.f, 1, EUMGSequencePlayMode::Reverse);
		}
	}

	FButtonStyle Style = UserButton->GetStyle();

	const FLinearColor BaseColor = IsActive ? ActiveColor : InactiveColor;

	Style.Normal.TintColor = FSlateColor(BaseColor);
	Style.Hovered.TintColor = FSlateColor(BaseColor);
	Style.Pressed.TintColor = FSlateColor(BaseColor);

	UserButton->SetStyle(Style);

	float BackgroundOpacity = IsActive ? 1.f : 0.f;

	ActiveBackgroundImage->SetOpacity(BackgroundOpacity);

}
