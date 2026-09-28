// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DC1SaveSubsystem.generated.h"

class ADC1PlayerState;
class UDC1PlayerSaveGame;

struct FDC1AppearanceData;

UCLASS()
class DC1_API UDC1SaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	bool SavePlayerData(ADC1PlayerState* PS);
	bool LoadPlayerData(ADC1PlayerState* PS);

	bool SaveCustomizeData(const FDC1AppearanceData& InData);
	bool LoadCustomizeData(FDC1AppearanceData* OutData);

public:
	// [Client] Customize Local File Check
	bool HasCustomizeLocalData();

public:
	static constexpr int32 CurrentSaveVersion = { 1 };

private:
	FString MakeSlotName(ADC1PlayerState* PS);

private:
	FString CustomizeFileName = { "Customize" };
};
