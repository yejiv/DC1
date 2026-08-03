// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DC1PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class DC1_API ADC1PlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ADC1PlayerController(const FObjectInitializer& ObjectInitializer);

public:
	virtual void BeginPlay() override;
};
