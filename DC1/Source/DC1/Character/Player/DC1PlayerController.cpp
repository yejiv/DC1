// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/DC1PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/Input/DC1InputData.h"

#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/Input/DC1GameplayTag_Input.h"

#include "Character/DC1Character.h"

ADC1PlayerController::ADC1PlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void ADC1PlayerController::SetCursorMode(bool OnTurn)
{
	IsTurn = OnTurn;
}

void ADC1PlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetUp_IMC();
}

void ADC1PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (const UDC1InputData* InputData = UDC1AssetManager::GetAssetByTag<UDC1InputData>(DC1AssetTags::Asset_InputData))
	{
		UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

		// Move
		if (const UInputAction* MoveAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_Move))
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::OnMove);
		}

		// Turn
		if (const UInputAction* TurnAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_Turn))
		{
			EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ThisClass::OnTurn);
		}
	}
}

void ADC1PlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	ControlObject = Cast<ADC1Character>(aPawn);
}

void ADC1PlayerController::OnMove(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}

	const FVector2D MoveVector = Value.Get<FVector2D>();

	FRotator YawRotator(0.f, GetControlRotation().Yaw, 0.f);

	const FVector Forward = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::Y);

	ControlObject->AddMovementInput(Forward);
	ControlObject->AddMovementInput(Right);
}

void ADC1PlayerController::OnTurn(const FInputActionValue& Value)
{
	if (false == IsTurn)
	{ 
		return; 
	}

	const FVector2D LookVector = Value.Get<FVector2D>();
	AddYawInput(LookVector.X);
	AddPitchInput(LookVector.Y);
}

void ADC1PlayerController::SetUp_IMC()
{

	if (const UDC1InputData* InputData = UDC1AssetManager::GetAssetByTag<UDC1InputData>(DC1AssetTags::Asset_InputData))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputData->FindInputMappingContextbyTag(DC1IMCTags::IMC_Default), 0);
		}
	}
}
