// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Button/AnimButton/Tab/TabHeader/DC1CraftTabHeaderButton.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDC1CraftTabHeaderButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (true == ::IsValid(ButtonIconImage) && false == IconTexture.IsNull())
	{
		ButtonIconImage->SetBrushFromSoftTexture(IconTexture);
	}
}

void UDC1CraftTabHeaderButton::ApplyState()
{
	Super::ApplyState();

	const bool IsActive = (ButtonState == EDC1TabButtonState::Active);

	if (true == ::IsValid(ButtonIconImage))
	{
		ButtonIconImage->SetColorAndOpacity(IsActive ? ActiveIconColor : InactiveIconColor);
	}

	if (true == ::IsValid(ButtonBottomImage))
	{
		ButtonBottomImage->SetColorAndOpacity(IsActive ? ActiveBottomColor : InactiveBottomColor);
	}

	if (true == ::IsValid(TextLabel))
	{
		TextLabel->SetVisibility(IsActive ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
}
