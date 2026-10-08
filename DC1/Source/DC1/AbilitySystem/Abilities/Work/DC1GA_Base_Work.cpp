// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Work/DC1GA_Base_Work.h"
#include "Tags/State/DC1GameplayTag_State.h"

UDC1GA_Base_Work::UDC1GA_Base_Work(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 건설 완료, 파괴 시점에 GA 취소를 위한 Tag
	ActivationOwnedTags.AddTag(DC1StateTags::State_Working);
}

void UDC1GA_Base_Work::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (false == CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (true == ActorInfo->IsLocallyControlled())
	{
		ADC1InteractBuildActor* WorkActor = FindWorkTarget();
		if (false == ::IsValid(WorkActor))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		StartWork(WorkActor);
	}

	OnWorkStarted();
}

void UDC1GA_Base_Work::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	const FGameplayAbilityActorInfo* Info = (nullptr == ActorInfo) ? GetCurrentActorInfo() : ActorInfo;

	if (nullptr != Info && true == Info->IsLocallyControlled())
	{
		StopWork();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
