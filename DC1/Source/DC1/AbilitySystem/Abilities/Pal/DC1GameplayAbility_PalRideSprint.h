// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "GameplayEffectTypes.h"
#include "DC1GameplayAbility_PalRideSprint.generated.h"

class ADC1PalCharacter;
class UGameplayEffect;

/**
 *
 */
UCLASS()
class DC1_API UDC1GameplayAbility_PalRideSprint : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	UDC1GameplayAbility_PalRideSprint(const FObjectInitializer& ObjectInitializer);

private:
	virtual bool	CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void	ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void	EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	void				HandleStaminaChanged(const FOnAttributeChangeData& Data);

	void				SetSprint(bool InSprint) const;

	ADC1PalCharacter*	GetPal() const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Cost")
	TSubclassOf<UGameplayEffect>	CostEffectClass = { nullptr };

private:
	FActiveGameplayEffectHandle		CostEffectHandle;
	FDelegateHandle					StaminaChangedHandle;
};
