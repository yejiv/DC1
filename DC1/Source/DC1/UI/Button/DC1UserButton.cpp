// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Button/DC1UserButton.h"

#include "UI/Tooltip/DC1TooltipWidget.h"

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

	if (nullptr != TooltipClass)
	{
		ToolTipWidgetDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(ThisClass, GetTooltipWidget));
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

void UDC1UserButton::RefreshTooltip(UDC1TooltipWidget* InTooltip)
{
}

UWidget* UDC1UserButton::GetTooltipWidget()
{
	if (false == ::IsValid(CachedTooltip))
	{
		CachedTooltip = CreateWidget<UDC1TooltipWidget>(GetOwningPlayer(), TooltipClass);
	}

	if (true == ::IsValid(CachedTooltip))
	{
		RefreshTooltip(CachedTooltip);
	}

	return CachedTooltip;
}
