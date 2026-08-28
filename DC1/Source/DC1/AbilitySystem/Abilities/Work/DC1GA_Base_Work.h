// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "DC1GA_Base_Work.generated.h"

class UDC1EquipComponent;

/**
 *
 */
UCLASS()
class DC1_API UDC1GA_Base_Work : public UDC1GameplayAbility
{
	GENERATED_BODY()
	
protected:
	virtual void			ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void			EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|EquipTool")
	FGameplayTag			EquipToolTag = { FGameplayTag::EmptyTag };

private:
	UDC1EquipComponent*		GetEquipComponent() const;
};
