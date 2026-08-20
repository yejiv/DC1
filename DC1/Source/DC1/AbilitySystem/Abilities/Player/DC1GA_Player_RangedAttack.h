// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/Abilities/Player/DC1GA_Player_Attack.h"
#include "GameplayTagContainer.h"
#include "DC1GA_Player_RangedAttack.generated.h"

class UAnimMontage;

UCLASS()
class DC1_API UDC1GA_Player_RangedAttack : public UDC1GA_Player_Attack
{
	GENERATED_BODY()

public:
	UDC1GA_Player_RangedAttack(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageAborted();

private:
	virtual void	ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Anim")
	FGameplayTag	DrawMontageTag;
};
