// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/DC1GA_Player_ClimbAtTop.h"

#include "Tags/DC1GamePlayTags.h"
#include "Character/Player/DC1Player.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/Anim/DC1AnimData.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"

#include "Component/DC1CharacterMovementComponent.h"

UDC1GA_Player_ClimbAtTop::UDC1GA_Player_ClimbAtTop(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationBlockedTags.AddTag(DC1StateTags::State_Dodging);
	ActivationBlockedTags.AddTag(DC1StateTags::State_Dead);
}

bool UDC1GA_Player_ClimbAtTop::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UDC1GA_Player_ClimbAtTop::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (false == CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (const UDC1AnimData* AnimData = UDC1AssetManager::GetAssetByTag<UDC1AnimData>(DC1AssetTags::Asset_Anim_Player))
	{
		if (UAnimMontage* Montage = AnimData->FindMontageByTag(DC1AnimTags::Anim_Action_ClimbAtTop))
		{
			if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, 1.f))
			{
				CMC = Cast<UDC1CharacterMovementComponent>(ActorInfo->MovementComponent.Get());

				FVector Target = {};
				if (false == CMC.IsValid() || false == CMC->FindMantleTarget(Target))
				{
					EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
					return;
				}

				CMC->SetMovementMode(MOVE_Flying);

				PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageAborted);
				PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageAborted);
				PlayMontageTask->ReadyForActivation();

				Duration = Montage->GetPlayLength();

				MantleTargetLocation = Target;
				const FVector ActorLocation = CMC->GetActorLocation();
				const FVector WallUp = CMC->GetClimbUpVector();
				const float DeltaZ = FMath::Max(0.f, MantleTargetLocation.Z - ActorLocation.Z);
				const float SlopeLength = DeltaZ / FMath::Max(FLT_EPSILON, WallUp.Z);
				UpTarget = ActorLocation + WallUp * SlopeLength;

				UAbilityTask_ApplyRootMotionMoveToForce* UpTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
					this, NAME_None, UpTarget, Duration * UpRatio,
					false, MOVE_Flying, false, nullptr,
					ERootMotionFinishVelocityMode::SetVelocity, FVector::ZeroVector, 0.f
				);

				if (IsValid(UpTask))
				{
					UpTask->OnTimedOut.AddDynamic(this, &ThisClass::OnUpFailed);
					UpTask->OnTimedOutAndDestinationReached.AddDynamic(this, &ThisClass::OnUpReached);
					UpTask->ReadyForActivation();
				}
				else
				{
					EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
				}
			}
		}
		else
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UDC1GA_Player_ClimbAtTop::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (CMC.IsValid() && MOVE_Flying == CMC->MovementMode)
	{
		CMC->SetMovementMode(bWasCancelled ? MOVE_Falling : MOVE_Walking);
	}
	CMC.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDC1GA_Player_ClimbAtTop::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UDC1GA_Player_ClimbAtTop::OnUpFailed()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UDC1GA_Player_ClimbAtTop::OnUpReached()
{
	UAbilityTask_ApplyRootMotionMoveToForce* ForwardTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
		this, NAME_None, MantleTargetLocation, Duration * (0.9f - UpRatio),
		false, MOVE_Flying, false, nullptr,
		ERootMotionFinishVelocityMode::SetVelocity, FVector::ZeroVector, 0.f
	);

	if (IsValid(ForwardTask))
	{
		ForwardTask->OnTimedOut.AddDynamic(this, &ThisClass::OnForwardFinished);
		ForwardTask->OnTimedOutAndDestinationReached.AddDynamic(this, &ThisClass::OnForwardFinished);
		ForwardTask->ReadyForActivation();
	}
}

void UDC1GA_Player_ClimbAtTop::OnForwardFinished()
{
	if(CMC.IsValid())
		CMC->SetMovementMode(MOVE_Walking);

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

ADC1Player* UDC1GA_Player_ClimbAtTop::GetPlayer() const
{
	return Cast<ADC1Player>(GetAvatarActorFromActorInfo());
}