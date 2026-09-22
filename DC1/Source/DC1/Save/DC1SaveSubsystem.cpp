// Fill out your copyright notice in the Description page of Project Settings.


#include "Save/DC1SaveSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "Define/DC1Macro.h"

#include "Save/DC1PlayerSaveGame.h"
#include "Character/Player/DC1PlayerState.h"

void UDC1SaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UDC1SaveSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UDC1SaveSubsystem::SavePlayerData(ADC1PlayerState* PS)
{
	if (false == IsValid(PS) || false == PS->HasAuthority())
		return false;

	UDC1PlayerSaveGame* SaveGame = Cast<UDC1PlayerSaveGame>(UGameplayStatics::CreateSaveGameObject(UDC1PlayerSaveGame::StaticClass()));
	if (false == IsValid(SaveGame))
	{
		LOG_ERROR(TEXT("Create SaveGame is Failed"))
		return false;
	}

	PS->WriteSaveData(SaveGame);

	FString SlotName = MakeSlotName(PS);
	if (SlotName.IsEmpty())
	{
		LOG_ERROR(TEXT("SlotName is Empty"))
		return false;
	}
	SaveGame->SetSaveVersion(CurrentSaveVersion);
	SaveGame->SetOwnerNetId(SlotName);

	return UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
}

bool UDC1SaveSubsystem::LoadPlayerData(ADC1PlayerState* PS)
{
	if (false == IsValid(PS) || false == PS->HasAuthority())
		return false;

	FString SlotName = MakeSlotName(PS);
	if (SlotName.IsEmpty())
	{
		return false;
	}

	if (false == UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		LOG(TEXT("[Load] Load File is not exist"));
		return false;
	}

	UDC1PlayerSaveGame* SaveGame = Cast<UDC1PlayerSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (true == IsValid(SaveGame))
	{
		PS->ReadSaveData(SaveGame);
	}
	else
	{
		// 신규 플레이어
	}

	// SaveVersion에 따른 분기 처리 (추후)

	return true;
}

FString UDC1SaveSubsystem::MakeSlotName(ADC1PlayerState* PS)
{
	// Null Subsystem 예외 처리
	const FUniqueNetIdRepl& NetId = PS->GetUniqueId();
	if (false == NetId.IsValid() || NetId.GetType() == NULL_SUBSYSTEM)
	{
		LOG_WARNING(TEXT("[Save Warning] Subsystem is Null"));
		return FString();
	}

	FString ID = TEXT("Player_");
	ID.Append(NetId.ToString());

	return ID;
}
