// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DC1SaveSubsystem.generated.h"

class ADC1PlayerState;
class UDC1PlayerSaveGame;

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

public:
	static constexpr int32 CurrentSaveVersion = { 1 };

private:
	FString MakeSlotName(ADC1PlayerState* PS);
};
