// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "DC1GA_Player_HitReact.generated.h"

class ADC1Player;
class UCurveFloat;

/**
 * 플레이어 피격 반응. 넉백 + 피격 GameplayCue(카메라 흔들림).
 */
UCLASS()
class DC1_API UDC1GA_Player_HitReact : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	UDC1GA_Player_HitReact(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION()
	void OnKnockbackFinished();

private:
	virtual void	ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void	EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	ADC1Player*		GetPlayer() const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Cue")
	FGameplayTag	HitCueTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Move")
	float				KnockbackStrength = { 600.f };
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Move")
	float				KnockbackDuration = { 0.1f };
	// 시간(0~1) 대비 세기 배율. 비우면 일정한 속도
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Move")
	TObjectPtr<UCurveFloat>	KnockbackStrengthCurve = { nullptr };
};
