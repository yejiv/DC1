// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Button/AnimButton/Tab/TabHeader/DC1BuildTabHeaderButton.h"

#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UDC1BuildTabHeaderButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (true == ::IsValid(ButtonIconImage) && false == IconTexture.IsNull())
	{
		ButtonIconImage->SetBrushFromSoftTexture(IconTexture);
	}
}

void UDC1BuildTabHeaderButton::ApplyState()
{
	Super::ApplyState();

	if (true == ::IsValid(RootOverlay))
	{
		float RootOpacity = ButtonState == EDC1TabButtonState::Disable ? DisableOpacity : 1.f;

		RootOverlay->SetRenderOpacity(RootOpacity);
	}

	if (true == ::IsValid(ActiveHoverImage))
	{
		FLinearColor HoverColor = (ButtonState == EDC1TabButtonState::Active) ? ActiveHoverColor : InactiveHoverColor;
		ActiveHoverImage->SetColorAndOpacity(HoverColor);
	}

	if (true == ::IsValid(ButtonIconImage) && true == ::IsValid(TextLabel))
	{
		FLinearColor Color = (ButtonState == EDC1TabButtonState::Active) ? ActiveFontIconColor : InactiveFontIconColor;

		ButtonIconImage->SetColorAndOpacity(Color);
		TextLabel->SetColorAndOpacity(Color);
	}
}

bool UDC1BuildTabHeaderButton::SetState(EDC1TabButtonState InState)
{
	EDC1TabButtonState CachedState = ButtonState;

	if (InState == EDC1TabButtonState::Active)
	{
		StopAnim(InactivedAnim);
	}

	if (false == Super::SetState(InState))
	{
		return false;
	}

	if (CachedState == EDC1TabButtonState::Active && InState == EDC1TabButtonState::Inactive)
	{
		FDC1AnimPlayInfo PlayInfo;

		PlayInfo.PlayMode = EUMGSequencePlayMode::Forward;
		PlayInfo.RestoreState = true;

		PlayAnim(InactivedAnim, PlayInfo);
	}


	return true;
}
