// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/DC1GameMode.h"

#include "Character/Player/DC1PlayerController.h"
#include "Character/Player/DC1PlayerState.h"
#include "Character/Player/DC1Player.h"

#include "Define/DC1Macro.h"
#include "Save/DC1SaveSubsystem.h"

ADC1GameMode::ADC1GameMode()
{
	PlayerControllerClass = ADC1PlayerController::StaticClass();
	PlayerStateClass = ADC1PlayerState::StaticClass();
	DefaultPawnClass = ADC1Player::StaticClass();
}

void ADC1GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ADC1PlayerState* PS = NewPlayer ? NewPlayer->GetPlayerState<ADC1PlayerState>() : nullptr;

	if (nullptr == PS)
	{
		return;
	}

	const bool bLoaded = GetGameInstance()->GetSubsystem<UDC1SaveSubsystem>()->LoadPlayerData(PS);

	if (false == bLoaded)
	{
		LOG_WARNING(TEXT("[Load] Load Failed"));
	}
}
