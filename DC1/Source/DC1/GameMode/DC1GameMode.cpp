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

void ADC1GameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UDC1SaveSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UDC1SaveSubsystem>())
	{
		Subsystem->StartAutoSave();
	}
}

void ADC1GameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UDC1SaveSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UDC1SaveSubsystem>())
	{
		Subsystem->StopAutoSave();
		Subsystem->SaveAll();
		Subsystem->FlushAll();
	}

	Super::EndPlay(EndPlayReason);
}

void ADC1GameMode::StartPlay()
{
	Super::StartPlay();

	for (ADC1PlayerState* PS : PendingLoadedStates)
	{
		if (false == IsValid(PS))
			continue;
		LoadPlayer(PS);
	}
	PendingLoadedStates.Reset();
}

void ADC1GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ADC1PlayerState* PS = NewPlayer ? NewPlayer->GetPlayerState<ADC1PlayerState>() : nullptr;

	if (false == IsValid(PS))
	{
		return;
	}

	// BeginPlay가 호출되기 전이면 Load 대기
	if (false == GetWorld()->HasBegunPlay())
	{
		PendingLoadedStates.Add(PS);
		return;
	}

	LoadPlayer(PS);
}

void ADC1GameMode::Logout(AController* Exiting)
{
	if (UDC1SaveSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UDC1SaveSubsystem>())
	{
		if (ADC1PlayerState* PS = Exiting ? Exiting->GetPlayerState<ADC1PlayerState>() : nullptr)
		{
			Subsystem->SavePlayerData(PS);
		}
	}

	Super::Logout(Exiting);
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

void ADC1GameMode::LoadPlayer(ADC1PlayerState* InPS)
{
	UDC1SaveSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UDC1SaveSubsystem>();
	if (nullptr == Subsystem)
		return;

	const bool bLoaded = Subsystem->LoadPlayerData(InPS);
	InPS->SetCompleteLoaded(bLoaded);

	if (false == bLoaded)
	{
		LOG_WARNING(TEXT("[Load] Load Failed"));
	}
}
