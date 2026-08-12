// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "DC1PlayerState.generated.h"

class UDC1AbilitySystemComponent;
class UDC1PlayerSet;
class UDC1TechTreeComponent;

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
	virtual UAbilitySystemComponent*	GetAbilitySystemComponent() const override;
	UDC1TechTreeComponent*				GetTechTreeComponent() const { return TechTreeComponent; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Ability")
	TObjectPtr<UDC1AbilitySystemComponent> AbilitySystemComponent = { nullptr };
	UPROPERTY()
	TObjectPtr<UDC1PlayerSet> PlayerSet = { nullptr };
	UPROPERTY(VisibleAnywhere, Category = "Tech")
	TObjectPtr< UDC1TechTreeComponent>		TechTreeComponent = { nullptr };
};
