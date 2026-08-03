// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/DC1GameMode.h"

#include "Character/Player/DC1PlayerController.h"
#include "Character/Player/DC1PlayerState.h"
#include "Character/Player/DC1Player.h"

ADC1GameMode::ADC1GameMode()
{
	PlayerControllerClass = ADC1PlayerController::StaticClass();
	PlayerStateClass = ADC1PlayerState::StaticClass();
	DefaultPawnClass = ADC1Player::StaticClass();
}
