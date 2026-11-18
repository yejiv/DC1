// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "DC1GameplayAbility_PalSleep.generated.h"

class UAbilityTask_PlayMontageAndWait;
struct FGameplayEventData;

/**
 * 
 */
UCLASS()
class DC1_API UDC1GameplayAbility_PalSleep : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	UDC1GameplayAbility_PalSleep(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// InMontageTag 의 몽타주를 튼다. 몽타주가 없으면 어빌리티를 끝낸다
	void PlaySleepMontage(const FGameplayTag& InMontageTag);

	UFUNCTION()
	void OnWake(FGameplayEventData InPayload);

	UFUNCTION()
	void OnMontageFinished();

	UFUNCTION()
	void OnMontageAborted();

protected:
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait>	MontageTask = { nullptr };

	// 지금 트는 몽타주
	FGameplayTag	CurrentMontageTag;

	bool			bWakeRequested = { false };
};
