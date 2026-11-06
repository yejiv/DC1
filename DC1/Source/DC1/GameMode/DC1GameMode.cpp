// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/DC1GameMode.h"

#include "Character/Player/DC1PlayerController.h"
#include "Character/Player/DC1PlayerState.h"
#include "Character/Player/DC1Player.h"
#include "GameMode/DC1GameState.h"

#include "Define/DC1Macro.h"
#include "Save/DC1SaveSubsystem.h"

ADC1GameMode::ADC1GameMode()
{
	PlayerControllerClass = ADC1PlayerController::StaticClass();
	PlayerStateClass = ADC1PlayerState::StaticClass();
	DefaultPawnClass = ADC1Player::StaticClass();
	GameStateClass = ADC1GameState::StaticClass();
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

bool ADC1GameMode::RespawnPlayer(AController* InController, const FTransform* InSpawnTransform)
{
	if (nullptr == InController)
		return false;

	ADC1PlayerState* PS = InController->GetPlayerState<ADC1PlayerState>();
	if (nullptr == PS)
		return false;

	// 기존 Pawn 정리 ( 남아 있으면 RestartPlayer가 새로 만들지 않고 그대로 재사용한다 )
	if (APawn* OldPawn = InController->GetPawn())
	{
		InController->UnPossess();
		OldPawn->Destroy();
	}

	// 새 Pawn이 상태 태그를 읽기 전에 초기화
	PS->ResetForRespawn(RespawnHealthRatio);

	if (nullptr != InSpawnTransform)
	{
		RestartPlayerAtTransform(InController, *InSpawnTransform);
	}
	else
	{
		RestartPlayer(InController);
	}

	return nullptr != InController->GetPawn();
}
