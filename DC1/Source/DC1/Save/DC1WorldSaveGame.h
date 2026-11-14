// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "Save/DC1SaveTypes.h"

#include "DC1WorldSaveGame.generated.h"

// 서버 월드 데이터 ( 설치된 액터들 )
UCLASS()
class DC1_API UDC1WorldSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	const int32			GetSaveVersion() const { return SaveVersion; }
	void					SetSaveVersion(const int32 In) { SaveVersion = In; }

public:
	UPROPERTY()
	TArray<FDC1ActorSaveRecord> BuildActors;

	UPROPERTY()
	TArray<FDC1ActorSaveRecord> PickupActors;

private:
	UPROPERTY()
	int32		SaveVersion = {};
};
