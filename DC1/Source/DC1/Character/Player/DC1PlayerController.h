// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "DC1PlayerController.generated.h"

class ADC1Character;
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
	void SetCursorMode(bool OnTurn);

public:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* aPawn) override;

private:
	void OnMove(const FInputActionValue& Value);
	void OnTurn(const FInputActionValue& Value);

private:
	// InputMappingContext SetUp
	void SetUp_IMC();

private:
	TObjectPtr<ADC1Character> ControlObject = { nullptr };

private:
	bool	IsTurn = { true };
};
