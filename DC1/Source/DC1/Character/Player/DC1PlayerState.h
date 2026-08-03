// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "DC1PlayerState.generated.h"

class UDC1AbilitySystemComponent;
/**
 * 
 */
UCLASS()
class DC1_API ADC1PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	ADC1PlayerState(const FObjectInitializer& ObjectInitializer);

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	TObjectPtr<UDC1AbilitySystemComponent> AbilitySystemComponent = { nullptr };
};
