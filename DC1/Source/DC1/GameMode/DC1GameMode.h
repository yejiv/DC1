// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DC1GameMode.generated.h"

/**
 * 
 */
UCLASS()
class DC1_API ADC1GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADC1GameMode();

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
