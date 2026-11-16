// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Define/DC1Enums.h"
#include "DC1SoundSaveGame.generated.h"

// SoundCategory별 조절값'만' 저장
// 클라 로컬 전용 ( CustomizationSaveGame과 같은 형태 )

UCLASS()
class DC1_API UDC1SoundSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<EDC1SoundCategory, float> Volumes;
};
