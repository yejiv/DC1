// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "DC1GA_Player_Attack.generated.h"

class UAnimMontage;
class UGameplayEffect;
/**
 * 
 */
UCLASS()
class DC1_API UDC1GA_Player_Attack : public UDC1GameplayAbility
{
	GENERATED_BODY()
	
public:
	UDC1GA_Player_Attack(const FObjectInitializer& ObjectInitializer);

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Anim")
	TObjectPtr<UAnimMontage>	AnimMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|EventTag")
	FGameplayTag						HitEventTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Effect")
	TSubclassOf<UGameplayEffect>	EffectClass;

private:
	TSet<TObjectPtr<const AActor>>		HitTargets;
};
