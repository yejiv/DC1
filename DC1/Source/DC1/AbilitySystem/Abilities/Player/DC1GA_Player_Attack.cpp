// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/DC1GA_Player_Attack.h"

#include "Animation/AnimMontage.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "AbilitySystem/Attribute/DC1PlayerSet.h"
#include "Tags/DC1GamePlayTags.h"

UDC1GA_Player_Attack::UDC1GA_Player_Attack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HitEventTag = DC1PlayerTags::Event_Player_Hit;
}

void UDC1GA_Player_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (false == CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Hit Event Bind
	if (HitEventTag.IsValid())
	{
		if (UAbilityTask_WaitGameplayEvent* HitEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, HitEventTag))
		{
			HitEvent->EventReceived.AddDynamic(this, &ThisClass::OnHitReceived);
			HitEvent->ReadyForActivation();
		}
	}

	// Montage Task Bind
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AnimMontage);
	if (nullptr == PlayMontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
	PlayMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageAborted);
	PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageAborted);
	PlayMontageTask->ReadyForActivation();

	// Hit Target Container Reset
	HitTargets.Reset();
}

void UDC1GA_Player_Attack::OnHitReceived(FGameplayEventData Payload)
{
	if (nullptr == CurrentActorInfo || false == CurrentActorInfo->IsNetAuthority()) return;

	const AActor* HitTarget = Payload.Target;
	if (nullptr == HitTarget || HitTargets.Contains(HitTarget)) return;
	HitTargets.Add(HitTarget);

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitTarget);

	if (nullptr == SourceASC || nullptr == TargetASC) return;

	// TODO : 팀필터 거르기

	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(EffectClass, GetAbilityLevel());
	if (false == Spec.IsValid()) return;

	float AttackPower = SourceASC->GetNumericAttribute(UDC1PlayerSet::GetAttackPowerAttribute());
	Spec.Data->SetSetByCallerMagnitude(DC1PlayerTags::SetByCaller_Damage, AttackPower);
	SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}

void UDC1GA_Player_Attack::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDC1GA_Player_Attack::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
