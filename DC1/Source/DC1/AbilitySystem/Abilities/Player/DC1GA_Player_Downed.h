// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "GameplayEffectTypes.h"
#include "DC1GA_Player_Downed.generated.h"

class ADC1Player;
class UGameplayEffect;

/**
 * 플레이어 기절. HP 0이 되면 서버가 활성화한다.
 * 살아 있는 동안 State.Downed가 붙고, 기절 게이지가 시간에 따라 줄어든다.
 * 팀원이 살리거나 사망하면 종료된다.
 */
UCLASS()
class DC1_API UDC1GA_Player_Downed : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	UDC1GA_Player_Downed(const FObjectInitializer& ObjectInitializer);

private:
	virtual void	ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void	EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	ADC1Player*		GetPlayer() const;

public:
	// 기절 게이지 시간 감소 ( Infinite + Period )
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Effect")
	TSubclassOf<UGameplayEffect>	DrainEffectClass;

private:
	FActiveGameplayEffectHandle		DrainEffectHandle;
};
