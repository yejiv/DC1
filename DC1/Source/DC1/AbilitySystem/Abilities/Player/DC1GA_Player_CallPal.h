// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "DC1GA_Player_CallPal.generated.h"

class UAnimMontage;

/**
 * 팰 소환/회수 ( E ).
 * 소환 중인 팰이 있으면 회수, 없으면 소환한다.
 */
UCLASS()
class DC1_API UDC1GA_Player_CallPal : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	UDC1GA_Player_CallPal(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageAborted();

private:
	virtual bool	CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void	ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void	EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

public:
	// 소환
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Anim")
	TObjectPtr<UAnimMontage>	DeployMontage = { nullptr };

	// 회수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Anim")
	TObjectPtr<UAnimMontage>	RetrieveMontage = { nullptr };

private:

};
