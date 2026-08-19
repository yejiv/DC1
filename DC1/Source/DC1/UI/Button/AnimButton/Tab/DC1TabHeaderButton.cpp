// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Button/AnimButton/Tab/DC1TabHeaderButton.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"

void UDC1TabHeaderButton::NativeConstruct()
{
	Super::NativeConstruct();

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

void UDC1TabHeaderButton::SetState(EDC1TabButtonState State)
{
	if (ButtonState == State)
	{
		return;
	}

	if (ButtonState == EDC1TabButtonState::Active || State == EDC1TabButtonState::Active)
	{
		if (true == ::IsValid(ClickedAnim))
		{
			EUMGSequencePlayMode::Type PlayMode = ButtonState == EDC1TabButtonState::Active ? EUMGSequencePlayMode::Reverse : EUMGSequencePlayMode::Forward;
			
			PlayAnimation(ClickedAnim, 0.f, 1, PlayMode);
		}
	}

	ButtonState = State;

	ApplyState();
}

void UDC1TabHeaderButton::ApplyState()
{
	if (false == ::IsValid(UserButton))
	{
		return;
	}

	FButtonStyle Style = UserButton->GetStyle();

	const FLinearColor BaseColor = ButtonState == EDC1TabButtonState::Active ? ActiveColor :
									ButtonState == EDC1TabButtonState::Inactive ? InactiveColor : DisableColor;

	Style.Normal.TintColor = FSlateColor(BaseColor);
	Style.Hovered.TintColor = FSlateColor(BaseColor);
	Style.Pressed.TintColor = FSlateColor(BaseColor);
	Style.Disabled.TintColor = FSlateColor(BaseColor);

	UserButton->SetStyle(Style);

	if (true == ::IsValid(ActiveBackgroundImage))
	{
		float BackgroundOpacity = ButtonState == EDC1TabButtonState::Active ? 1.f : 0.f;

		ActiveBackgroundImage->SetOpacity(BackgroundOpacity);
	}

}
