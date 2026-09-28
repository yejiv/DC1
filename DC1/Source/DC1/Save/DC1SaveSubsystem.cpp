// Fill out your copyright notice in the Description page of Project Settings.


#include "Save/DC1SaveSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "Define/DC1Macro.h"

#include "Save/DC1PlayerSaveGame.h"
#include "Save/DC1CustomizationSaveGame.h"
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
		// 신규 플레이어
		LOG(TEXT("[Load] New Player"));
		return true;
	}

	UDC1PlayerSaveGame* SaveGame = Cast<UDC1PlayerSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (true == IsValid(SaveGame))
	{
		PS->ReadSaveData(SaveGame);
	}
	else
	{
		return false;
	}

	// SaveVersion에 따른 분기 처리 (추후)

	return true;
}

bool UDC1SaveSubsystem::SaveCustomizeData(const FDC1AppearanceData& InData)
{
	UDC1CustomizationSaveGame* SaveGame = Cast<UDC1CustomizationSaveGame>(UGameplayStatics::CreateSaveGameObject(UDC1CustomizationSaveGame::StaticClass()));
	if (false == IsValid(SaveGame))
	{
		LOG_ERROR(TEXT("Create SaveGame is Failed"))
			return false;
	}

	SaveGame->Appearance = InData;

	return UGameplayStatics::SaveGameToSlot(SaveGame, CustomizeFileName, 0);
}

bool UDC1SaveSubsystem::LoadCustomizeData(FDC1AppearanceData* OutData)
{
	if (nullptr == OutData)
		return false;

	if (false == HasCustomizeLocalData())
	{
		return false;
	}

	UDC1CustomizationSaveGame* SaveGame = Cast<UDC1CustomizationSaveGame>(UGameplayStatics::LoadGameFromSlot(CustomizeFileName, 0));

	if (true == IsValid(SaveGame))
	{
		*OutData = SaveGame->Appearance;
	}

	return true;
}

bool UDC1SaveSubsystem::HasCustomizeLocalData()
{
	if (false == UGameplayStatics::DoesSaveGameExist(CustomizeFileName, 0))
		return false;

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
