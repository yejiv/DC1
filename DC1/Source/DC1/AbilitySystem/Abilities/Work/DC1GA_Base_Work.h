// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/DC1GameplayAbility.h"

#include "Actor/BuildActor/Interact/DC1InteractBuildActor.h"

#include "DC1GA_Base_Work.generated.h"

UCLASS(Abstract)
class DC1_API UDC1GA_Base_Work : public UDC1GameplayAbility
{
	GENERATED_BODY()


public:
	UDC1GA_Base_Work(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void			ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void			EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

// PURE_VIRTUAL
protected:
	// Work 대상 탐색 기능 구현
	virtual ADC1InteractBuildActor* FindWorkTarget() const	PURE_VIRTUAL(UDC1GA_Base_Work::FindWorkTarget, return nullptr;);

	// ADC1InteractBuildActor->AddWorker를 서버 측에서 호출하는 기능 구현
	virtual void			StartWork(ADC1InteractBuildActor* InWorkTarget) PURE_VIRTUAL(UDC1GA_Base_Work::StartWork, );

	// ADC1InteractBuildActor->RemoveWorker를 서버 측에서 호출하는 기능 구현
	virtual void			StopWork()	PURE_VIRTUAL(UDC1GA_Base_Work::StopWork, );

	// Work 중 액션 - ex) PlayMontage
	virtual void			OnWorkStarted() {};
};
