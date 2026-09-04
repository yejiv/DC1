// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Work/DC1GA_Base_Work.h"

#include "Actor/BuildActor/Interact/DC1InteractBuildActor.h"

#include "DC1GA_Base_BuildWork.generated.h"

UCLASS(Abstract)
class DC1_API UDC1GA_Base_BuildWork : public UDC1GA_Base_Work
{
	GENERATED_BODY()
	

public:
	UDC1GA_Base_BuildWork(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void			ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void			EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

// PURE_VIRTUAL
protected:
	// BuildActor 탐색 기능 구현
	virtual ADC1InteractBuildActor* FindBuildTarget() const	PURE_VIRTUAL(UDC1GA_Base_BuildWork::FindBuildTarget, return nullptr;);

	// ADC1InteractBuildActor->AddWorker를 서버 측에서 호출하는 기능 구현
	virtual void			StartWork(ADC1InteractBuildActor* InBuildTarget) PURE_VIRTUAL(UDC1GA_Base_BuildWork::StartWork, );

	// ADC1InteractBuildActor->RemoveWorker를 서버 측에서 호출하는 기능 구현
	virtual void			StopWork()	PURE_VIRTUAL(UDC1GA_Base_BuildWork::StopWork, );

	// Work 중 액션 - ex) PlayMontage
	virtual void			OnWorkStarted() {};
};
