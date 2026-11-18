// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/DC1GA_Player_Dodge.h"

#include "Tags/DC1GamePlayTags.h"
#include "Character/Player/DC1Player.h"
#include "Component/DC1CharacterMovementComponent.h"
#include "Component/DC1AbilitySystemComponent.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/Anim/DC1AnimData.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"

UDC1GA_Player_Dodge::UDC1GA_Player_Dodge(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RollMontageTag = DC1AnimTags::Anim_Action_Dodge_Roll;
	StepMontageTag = DC1AnimTags::Anim_Action_Dodge_Step;

	// 어빌리티 수명 = 회피 동작 전체. i-frame을 좁힐 땐 Invincible만 떼어내면 된다
	ActivationOwnedTags.AddTag(DC1StateTags::State_Dodging);
	ActivationOwnedTags.AddTag(DC1StateTags::State_Invincible);

	// 회피 중 재진입 금지
	ActivationBlockedTags.AddTag(DC1StateTags::State_Dodging);
	ActivationBlockedTags.AddTag(DC1StateTags::State_Dead);
	// 벽타기 중 회피 금지(bUseControllerRotationYaw로 벽을 등지게 된다)
	ActivationBlockedTags.AddTag(DC1StateTags::State_Climbing);
}

bool UDC1GA_Player_Dodge::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (false == Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check Grounded
	if (ADC1Player* Player = GetPlayer())
	{
		if (true == Player->GetCharacterMovement()->IsFalling())
			return false;
	}

	return true;
}

void UDC1GA_Player_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (false == CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	IsAiming = CheckAiming();

	// 진행 중인 공격 캔슬 (ASC->CancelAbilityByTag)
	if (ADC1Player* Player = GetPlayer())
	{
		if (UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(Player->GetAbilitySystemComponent()))
		{
			ASC->CancelAbilityByTag(DC1PlayerTags::Ability_Player_Attack);
		}
	}

	// Compute Input Direction
	Compute_InputDirection();
	// 기본 상태일 때, 입력 방향 바라보도록 전환
	if (false == IsAiming)
	{
		FaceInputDirection();
		Strength = RollStrength;
	}
	else
		Strength = StepStrength;
	
	// SelectMontageTag / SelectStepSection으로 PlayMontageAndWait 실행
	if (const UDC1AnimData* AnimData = UDC1AssetManager::GetAssetByTag<UDC1AnimData>(DC1AssetTags::Asset_Anim_Player))
	{
		if (UAnimMontage* Montage = AnimData->FindMontageByTag(SelectMontageTag()))
		{
			if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, 1.f, SelectStepSection()))
			{
				PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
				PlayMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
				PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageAborted);
				PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageAborted);
				PlayMontageTask->ReadyForActivation();

				if (true == IsAiming)
				{
					if (ADC1Player* Player = GetPlayer())
					{
						Player->bUseControllerRotationYaw = true;
						Player->GetCharacterMovement()->bOrientRotationToMovement = false;
					}
				}

				UAbilityTask_ApplyRootMotionConstantForce* MoveTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
					this, NAME_None, InputWorldDirection, Strength, Duration, false, 
					nullptr,
					ERootMotionFinishVelocityMode::ClampVelocity,
					FVector::ZeroVector,
					2.f,
					true
				);

				if (nullptr != MoveTask)
					MoveTask->ReadyForActivation();
			}
		}
		else
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UDC1GA_Player_Dodge::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (ADC1Player* Player = GetPlayer())
	{
		Player->bUseControllerRotationYaw = false;
		Player->GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

FGameplayTag UDC1GA_Player_Dodge::SelectMontageTag() const
{
	return true == IsAiming ? StepMontageTag : RollMontageTag;
}

FName UDC1GA_Player_Dodge::SelectStepSection() const
{
	if (false == IsAiming)
		return NAME_None;

	if (FMath::Abs(InputLocalDirection.Y) > FMath::Abs(InputLocalDirection.X))
	{
		if (InputLocalDirection.Y > 0)
		{
			return "Right";
		}
		else if (InputLocalDirection.Y < 0)
		{
			return "Left";
		}
	}
	else
	{
		if (InputLocalDirection.X > 0)
		{
			return "Forward";
		}
		else if (InputLocalDirection.X < 0)
		{
			return "Backward";
		}
	}

	return "Backward";
}

void UDC1GA_Player_Dodge::Compute_InputDirection()
{
	if (ADC1Player* Player = GetPlayer())
	{
		if (UCharacterMovementComponent* CMC = Player->GetCharacterMovement())
		{
			InputWorldDirection = CMC->GetCurrentAcceleration().GetSafeNormal2D();
			InputLocalDirection = Player->GetActorRotation().UnrotateVector(InputWorldDirection);
		}
	}
}

void UDC1GA_Player_Dodge::FaceInputDirection()
{
	if (InputWorldDirection.IsNearlyZero())
		return;

	if (ADC1Player* Player = GetPlayer())
	{
		Player->SetActorRotation(FRotator(0.f, InputWorldDirection.Rotation().Yaw, 0.f));
	}
}

ADC1Player* UDC1GA_Player_Dodge::GetPlayer() const
{
	return Cast<ADC1Player>(GetAvatarActorFromActorInfo());
}

bool UDC1GA_Player_Dodge::CheckAiming() const
{
	if (ADC1Player* Player = GetPlayer())
	{
		if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
		{
			if (true == ASC->HasMatchingGameplayTag(DC1StateTags::State_Aiming))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}

void UDC1GA_Player_Dodge::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDC1GA_Player_Dodge::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}