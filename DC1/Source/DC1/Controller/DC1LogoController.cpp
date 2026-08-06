// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/DC1LogoController.h"
#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Tags/UI/DC1GameplayTag_UI.h"

void ADC1LogoController::BeginPlay()
{
	Super::BeginPlay();

	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
		return;
	}

	UIManager->PushScreen(DC1UITags::UI_Logo_Title);
}
