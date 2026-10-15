// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/DC1GA_Player_HitReact.h"

#include "Character/Player/DC1Player.h"

#include "Tags/DC1GamePlayTags.h"

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"

UDC1GA_Player_HitReact::UDC1GA_Player_HitReact(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	bRetriggerInstancedAbility = true;

	FAbilityTriggerData Trigger = {};
	Trigger.TriggerTag = DC1PlayerTags::Event_Player_Damaged;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UDC1GA_Player_HitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (nullptr == TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}


	FVector KnockbackDirection = {};
	FVector OwnerLocation = GetAvatarActorFromActorInfo()->GetActorLocation();

	// 1. HitResult
	if (const FHitResult* Hit = TriggerEventData->ContextHandle.GetHitResult())
	{
		FVector ImpactLocation = Hit->ImpactPoint;

		KnockbackDirection = OwnerLocation - ImpactLocation;
		KnockbackDirection = KnockbackDirection.GetSafeNormal();
	}
	// 2. Owner - Causer (Location)
	else if(nullptr != TriggerEventData->Instigator)
	{
		FVector CauserLocation = TriggerEventData->Instigator->GetActorLocation();

		KnockbackDirection = OwnerLocation - CauserLocation;
		KnockbackDirection.Z = 0.f;
		KnockbackDirection = KnockbackDirection.GetSafeNormal2D();
	}
	// 3. Owner Backward Knockback
	else
	{
		KnockbackDirection = -(GetAvatarActorFromActorInfo()->GetActorForwardVector().GetSafeNormal2D());
	}

	UAbilityTask_ApplyRootMotionConstantForce* KnockbackTask = 
		UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(this, NAME_None, 
			KnockbackDirection, KnockbackStrength, KnockbackDuration, 
			false, KnockbackStrengthCurve,
			ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity, 
			FVector::ZeroVector, 0.f, true
		);

	if (false == IsValid(KnockbackTask))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	KnockbackTask->OnFinish.AddDynamic(this, &ThisClass::OnKnockbackFinished);
	KnockbackTask->ReadyForActivation();

	// GameplayCue (Shaking)
	K2_ExecuteGameplayCue(HitCueTag, TriggerEventData->ContextHandle);
}

void UDC1GA_Player_HitReact::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDC1GA_Player_HitReact::OnKnockbackFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

ADC1Player* UDC1GA_Player_HitReact::GetPlayer() const
{
	return Cast<ADC1Player>(GetAvatarActorFromActorInfo());
}
