// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "Data/Pal/DC1PalTypes.h"
#include "Save/DC1SaveTypes.h"

#include "DC1PlayerSaveGame.generated.h"

UCLASS()
class DC1_API UDC1PlayerSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	const int32			GetSaveVersion() const { return SaveVersion; }
	void					SetSaveVersion(const int32 In) { SaveVersion = In; }

	const FString		GetOwnerNetId() const { return OwnerNetId; }
	void					SetOwnerNetId(const FString& In) { OwnerNetId = In; }

public:
	UPROPERTY()
	TArray<FDC1ObjectSaveRecord> Components;

private:
	UPROPERTY()
	int32		SaveVersion = {};

	UPROPERTY()
	FString	OwnerNetId = {};
};
