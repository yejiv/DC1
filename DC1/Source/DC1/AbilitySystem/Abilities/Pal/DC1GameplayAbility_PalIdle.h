// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "DC1GameplayAbility_PalIdle.generated.h"

/**
 * 
 * 
 */
UCLASS()
class DC1_API UDC1GameplayAbility_PalIdle : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	UDC1GameplayAbility_PalIdle(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION()
	void OnMontageFinished();

	UFUNCTION()
	void OnMontageAborted();
};
