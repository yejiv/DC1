// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DC1UserWidget.h"
#include "Animation/WidgetAnimation.h"

void UDC1UserWidget::Activate()
{
	if (true == :: IsValid(IntroAnim))
	{
		PlayAnimation(IntroAnim);
	}

	NativeOnActivated();
}

void UDC1UserWidget::Deactivate()
{
	if (true == ::IsValid(OutroAnim))
	{
		PlayAnimation(OutroAnim);
	}

	NativeOnDeactivated();
}

void UDC1UserWidget::HandleInputKey(const FKey& InputKey)
{
}

void UDC1UserWidget::NativeOnActivated()
{
}

void UDC1UserWidget::NativeOnDeactivated()
{
}