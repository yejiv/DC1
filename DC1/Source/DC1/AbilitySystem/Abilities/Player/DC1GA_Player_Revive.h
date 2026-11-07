// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "DC1GA_Player_Revive.generated.h"

class ADC1Player;

/**
 * 기절한 팀원 치료. 상호작용 F 홀드 동안 실행된다.
 */
UCLASS()
class DC1_API UDC1GA_Player_Revive : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	UDC1GA_Player_Revive(const FObjectInitializer& ObjectInitializer);

private:
	virtual bool	CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void	ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void	EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UFUNCTION()
	void HandleInputReleased(float TimeHeld);
	UFUNCTION()
	void HandleReviveDelayFinished();

	void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag InActivationTag);

private:
	void WaitReviveTarget();
	void BeginRevive(ADC1Player* InTarget);
	void SendReviveTarget();

private:
	ADC1Player* FindReviveTarget(const FGameplayAbilityActorInfo* ActorInfo) const;

private:
	// 치료 대상
	TWeakObjectPtr<ADC1Player>	ReviveTarget = { nullptr };

private:
	FDelegateHandle		TargetDataSetHandle;
};
