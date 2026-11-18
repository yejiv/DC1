// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "DC1GA_Player_ClimbAtTop.generated.h"

class ADC1Player;
class UDC1CharacterMovementComponent;

/**
 * 벽 꼭대기 / 낮은 벽 올라서기.
 */
UCLASS()
class DC1_API UDC1GA_Player_ClimbAtTop : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	UDC1GA_Player_ClimbAtTop(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION()
	void OnMontageAborted();

	UFUNCTION()
	void OnUpFailed();
	UFUNCTION()
	void OnUpReached();
	UFUNCTION()
	void OnForwardFinished();

private:
	virtual bool	CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void	ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void	EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	ADC1Player*		GetPlayer() const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Anim|Move")
	float			UpRatio = { 0.6f };

private:
	UPROPERTY()
	TWeakObjectPtr<UDC1CharacterMovementComponent>	CMC = { nullptr };
	
private:
	FVector		UpTarget = {};
	FVector		MantleTargetLocation = {};
	float			Duration = {};
};
