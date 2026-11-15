// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/GameMenu/DC1GameMenuWidget.h"
#include "UI/Button/DC1UserButton.h"
#include "Character/Player/DC1PlayerController.h"

void UDC1GameMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Option->OnClickedDelegate.AddUObject(this, &ThisClass::HandleOptionClicked);
	Button_Title->OnClickedDelegate.AddUObject(this, &ThisClass::HandleTitleClicked);

	OptionWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UDC1GameMenuWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	OptionWidget->Deactivate();
	OptionWidget->SetVisibility(ESlateVisibility::Collapsed);
}

bool UDC1GameMenuWidget::ShouldPropagateTo(const UDC1UserWidget* InChild) const
{
	if (OptionWidget.Get() == InChild)
	{
		return false;
	}

	return Super::ShouldPropagateTo(InChild);
}

void UDC1GameMenuWidget::HandleOptionClicked()
{
	OptionWidget->SetVisibility(ESlateVisibility::Visible);
	OptionWidget->Activate();
}

void UDC1GameMenuWidget::HandleTitleClicked()
{
	ADC1PlayerController* PC = GetOwningPlayer<ADC1PlayerController>();
	if (nullptr == PC)
	{
		return;
	}

	PC->ReturnToTitle();
}
