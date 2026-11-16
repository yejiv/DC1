// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Button/AnimButton/GameMenu/DC1GameMenuButton.h"
#include "Components/Border.h"

void UDC1GameMenuButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (true == ::IsValid(ButtonBox))
	{
		ButtonBox->SetContentColorAndOpacity(UnhoverColor);
	}
}

void UDC1GameMenuButton::HandleHovered()
{
	Super::HandleHovered();

	if (true == ::IsValid(ButtonBox))
	{
		ButtonBox->SetContentColorAndOpacity(HoverColor);
	}
}

void UDC1GameMenuButton::HandleUnhovered()
{
	Super::HandleUnhovered();

	if (true == ::IsValid(ButtonBox))
	{
		ButtonBox->SetContentColorAndOpacity(UnhoverColor);
	}
}
