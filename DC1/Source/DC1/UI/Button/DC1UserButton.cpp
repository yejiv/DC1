// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Button/DC1UserButton.h"
#include "Components/Button.h"

void UDC1UserButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (true == ::IsValid(UserButton))
	{
		UserButton->OnClicked.AddDynamic(this, &ThisClass::HandleClicked);
		UserButton->OnHovered.AddDynamic(this, &ThisClass::HandleHovered);
		UserButton->OnUnhovered.AddDynamic(this, &ThisClass::HandleUnhovered);
	}
}

void UDC1UserButton::HandleClicked()
{
	OnClickedDelegate.Broadcast();
}

void UDC1UserButton::HandleHovered()
{
}

void UDC1UserButton::HandleUnhovered()
{
}
