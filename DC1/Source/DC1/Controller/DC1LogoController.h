// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DC1LogoController.generated.h"

/**
 * 
 */
UCLASS()
class DC1_API ADC1LogoController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:
	void TravelToServer(const FString& IP);
};
