// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/DC1GA_Player_MeleeAttack.h"

#include "Animation/AnimMontage.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "Character/DC1Character.h"
#include "Tags/DC1GamePlayTags.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/Anim/DC1AnimData.h"

UDC1GA_Player_MeleeAttack::UDC1GA_Player_MeleeAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HitEventTag = DC1PlayerTags::Event_Player_Hit;

	SectionName.Add("Attack_R");
	SectionName.Add("Attack_L");
}

void UDC1GA_Player_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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
	if (const UDC1AnimData* AnimData = UDC1AssetManager::GetAssetByTag<UDC1AnimData>(DC1AssetTags::Asset_Anim_Player))
	{
		UDC1AbilitySystemComponent* SourceASC = Cast<UDC1AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
		FGameplayTag StateTag = SourceASC->GetExclusiveStateTag(DC1StateTags::State_Weapon);
		if (nullptr == MappingStateToAnim.Find(StateTag))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
		if (UAnimMontage* Montage = AnimData->FindMontageByTag(MappingStateToAnim[StateTag]))
		{
			if (GetWorld()->GetTimeSeconds() - LastAttackTime > ResetTime)
				SectionIndex = 0;

			UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, 1.f, SectionName[SectionIndex]);
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

			SectionIndex = (SectionIndex + 1) % SectionName.Num();

			LastAttackTime = GetWorld()->GetTimeSeconds();
		}
	}
}

void UDC1GA_Player_MeleeAttack::OnHitReceived(FGameplayEventData Payload)
{
	if (nullptr == CurrentActorInfo || false == CurrentActorInfo->IsNetAuthority()) return;

	// 판정 창 단위 중복 제거는 보내는 쪽(ADC1Player)이 한다
	// TODO : 팀필터 거르기
	ApplyDamage(Payload.Target);
}

void UDC1GA_Player_MeleeAttack::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDC1GA_Player_MeleeAttack::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
