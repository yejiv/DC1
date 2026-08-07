// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Title/DC1TitleWidget.h"
#include "UI/Title/DC1TitleButton.h"
#include "UI/Title/DC1BracketButton.h"

#include "Components/WidgetSwitcher.h"
#include "Components/EditableTextBox.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "Controller/DC1LogoController.h"

#include "Helper/DC1Macro.h"

void UDC1TitleWidget::NativeConstruct()
{
	Super::NativeConstruct();

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

void UDC1TitleWidget::OnConnectClicked()
{
	if (false == ::IsValid(IPTextBox))
	{
		return;
	}
	
	ADC1LogoController* LogoController = Cast<ADC1LogoController>(UGameplayStatics::GetPlayerController(this, 0));
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
