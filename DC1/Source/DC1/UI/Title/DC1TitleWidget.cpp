// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Title/DC1TitleWidget.h"
#include "UI/Button/Title/DC1TitleButton.h"
#include "UI/Button/AnimButton/DC1BracketButton.h"
#include "UI/Tab/GameMenu/DC1SoundOptionWidget.h"

#include "Components/WidgetSwitcher.h"
#include "Components/EditableTextBox.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Controller/DC1LogoController.h"

#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Tags/UI/DC1GameplayTag_UI.h"

void UDC1TitleWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (true == ::IsValid(StartButton))
	{
		StartButton->OnClickedDelegate.AddUObject(this, &UDC1TitleWidget::OnStartClicked);
	}

	if (true == ::IsValid(QuitButton))
	{
		QuitButton->OnClickedDelegate.AddUObject(this, &UDC1TitleWidget::OnQuitClicked);
	}

	if (true == ::IsValid(ReturnButton))
	{
		ReturnButton->OnClickedDelegate.AddUObject(this, &UDC1TitleWidget::OnReturnClicked);
	}

	if (true == ::IsValid(ConnectButton))
	{
		ConnectButton->OnClickedDelegate.AddUObject(this, &UDC1TitleWidget::OnConnectClicked);
	}

	if (true == ::IsValid(OptionButton))
	{
		OptionButton->OnClickedDelegate.AddUObject(this, &UDC1TitleWidget::OnOptionClicked);
	}

	if (true == ::IsValid(OptionReturnButton))
	{
		OptionReturnButton->OnClickedDelegate.AddUObject(this, &UDC1TitleWidget::OnOptionReturnClicked);
	}
}

void UDC1TitleWidget::NativeConstruct()
{
	Super::NativeConstruct();

}

void UDC1TitleWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if (true == ::IsValid(SoundOptionWidget))
	{
		SoundOptionWidget->Deactivate();
	}

	if (true == ::IsValid(MenuSwitcher))
	{
		MenuSwitcher->SetActiveWidgetIndex(static_cast<int32>(EDC1Title::Main));
	}
}

bool UDC1TitleWidget::ShouldPropagateTo(const UDC1UserWidget* InChild) const
{
	if (SoundOptionWidget.Get() == InChild)
	{
		return false;
	}

	return Super::ShouldPropagateTo(InChild);
}

void UDC1TitleWidget::OnStartClicked()
{
	if (false == ::IsValid(MenuSwitcher))
	{
		return;
	}

	MenuSwitcher->SetActiveWidgetIndex(static_cast<int32>(EDC1Title::Connect));
}

void UDC1TitleWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, true);
}

void UDC1TitleWidget::OnReturnClicked()
{
	if (false == ::IsValid(MenuSwitcher))
	{
		return;
	}

	MenuSwitcher->SetActiveWidgetIndex(static_cast<int32>(EDC1Title::Main));
}

void UDC1TitleWidget::OnOptionClicked()
{
	if (false == ::IsValid(MenuSwitcher) || false == ::IsValid(SoundOptionWidget))
	{
		return;
	}

	MenuSwitcher->SetActiveWidgetIndex(static_cast<int32>(EDC1Title::Option));
	SoundOptionWidget->Activate();
}

void UDC1TitleWidget::OnOptionReturnClicked()
{
	if (true == ::IsValid(SoundOptionWidget))
	{
		SoundOptionWidget->Deactivate();
	}

	if (false == ::IsValid(MenuSwitcher))
	{
		return;
	}

	MenuSwitcher->SetActiveWidgetIndex(static_cast<int32>(EDC1Title::Main));
}

void UDC1TitleWidget::OnConnectClicked()
{
	if (false == ::IsValid(IPTextBox))
	{
		return;
	}
	
	ADC1LogoController* LogoController = Cast<ADC1LogoController>(GetOwningPlayer());
	if (false == ::IsValid(LogoController))
	{
		return;
	}

	FString IP = IPTextBox->GetText().ToString();
	if (true == IP.IsEmpty())
	{
		return;
	}

	LogoController->TravelToServer(IP);
}
