// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/DC1GameMode.h"

#include "Character/Player/DC1PlayerController.h"
#include "Character/Player/DC1PlayerState.h"
#include "Character/Player/DC1Player.h"

#include "Define/DC1Macro.h"

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

	const FUniqueNetIdRepl& NetId = PS->GetUniqueId();
	LOG(TEXT("[PostLogin] Name=%s Type=%s Id=%s Valid=%d"),
		*PS->GetPlayerName(),
		*NetId.GetType().ToString(),
		NetId.IsValid() ? *NetId.ToString() : TEXT("Invalid"),
		NetId.IsValid());
}
