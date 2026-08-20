// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/Abilities/Player/DC1GA_Player_Attack.h"
#include "GameplayTagContainer.h"
#include "DC1GA_Player_MeleeAttack.generated.h"

class UAnimMontage;

UCLASS()
class DC1_API UDC1GA_Player_MeleeAttack : public UDC1GA_Player_Attack
{
	GENERATED_BODY()

public:
	UDC1GA_Player_MeleeAttack(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION()
	void OnHitReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageFinished();
	UFUNCTION()
	void OnMontageAborted();

private:
	virtual void		ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|EventTag")
	FGameplayTag						HitEventTag;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Anim")
	TMap<FGameplayTag, FGameplayTag> MappingStateToAnim;

private:
	TArray<FName>					SectionName;

	uint32									SectionIndex = {};

	float									LastAttackTime = {};
	float									ResetTime = { 2.f };
};
