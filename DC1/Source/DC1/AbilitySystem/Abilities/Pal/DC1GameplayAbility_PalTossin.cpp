// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalTossin.h"

#include "Controller/DC1PalAIController.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "GameFramework/RootMotionSource.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

UDC1GameplayAbility_PalTossin::UDC1GameplayAbility_PalTossin(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationOwnedTags.AddTag(DC1PalTags::State_Pal_Attacking);

	MontageTag = DC1PalTags::Anim_Pal_Tossin;
	TriggerEventTag = DC1PalTags::Event_Pal_Attack_Hit;

	MaxRange = 900.f;
	TraceRadius = 70.f;
}

void UDC1GameplayAbility_PalTossin::PerformSkillEffect()
{
	APawn* Pal = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (false == ::IsValid(Pal))
	{
		return;
	}

	UWorld* World = Pal->GetWorld();
	if (nullptr == World)
	{
		return;
	}

	const FVector ChargeDirection = Pal->GetActorForwardVector().GetSafeNormal2D();

	if (AAIController* AIController = Cast<AAIController>(Pal->GetController()))
	{
		AIController->StopMovement();
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	DamagedActors.Reset();

	UAbilityTask_ApplyRootMotionConstantForce* ChargeTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
		this, NAME_None, ChargeDirection, ChargeSpeed, ChargeDuration,
		false,											// RVO 회피 우회한다
		nullptr,										// StrengthOverTime
		ERootMotionFinishVelocityMode::ClampVelocity,
		FVector::ZeroVector,
		0.f,											// 끝나면 즉시 정지
		true);											// bEnableGravity

	if (nullptr != ChargeTask)
	{
		ChargeTask->OnFinish.AddDynamic(this, &ThisClass::StopCharge);
		ChargeTask->ReadyForActivation();
	}

	World->GetTimerManager().SetTimer(SweepTimerHandle, this, &ThisClass::SweepDuringCharge, SweepInterval, true);
}

void UDC1GameplayAbility_PalTossin::SweepDuringCharge()
{
	SweepAndApplyDamage(ChargeTraceLength, DamagedActors);
}

void UDC1GameplayAbility_PalTossin::StopCharge()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SweepTimerHandle);
	}
}

void UDC1GameplayAbility_PalTossin::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	StopCharge();

	if (const APawn* Pal = Cast<APawn>(GetAvatarActorFromActorInfo()))
	{
		if (ADC1PalAIController* AIController = Cast<ADC1PalAIController>(Pal->GetController()))
		{
			if (AActor* Target = AIController->GetCurrentTarget())
			{
				AIController->SetFocus(Target, EAIFocusPriority::Gameplay);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
