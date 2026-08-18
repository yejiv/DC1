// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/DC1GA_Player_Attack.h"

#include "Animation/AnimMontage.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "AbilitySystem/Attribute/DC1PlayerSet.h"
#include "Character/DC1Character.h"
#include "Tags/DC1GamePlayTags.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/Anim/DC1AnimData.h"

UDC1GA_Player_Attack::UDC1GA_Player_Attack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	HitEventTag = DC1PlayerTags::Event_Player_Hit;

	SectionName.Add("Attack_R");
	SectionName.Add("Attack_L");
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

void UDC1GA_Player_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// NotifyEnd를 못 타고 끝나는 경로(캔슬, 피격 중단, 사망)를 위한 안전망
	if (ADC1Character* Character = Cast<ADC1Character>(GetAvatarActorFromActorInfo()))
	{
		Character->SetAttackCollision(false);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDC1GA_Player_Attack::OnHitReceived(FGameplayEventData Payload)
{
	if (nullptr == CurrentActorInfo || false == CurrentActorInfo->IsNetAuthority()) return;

	// 판정 창 단위 중복 제거는 보내는 쪽(ADC1Player)이 한다
	const AActor* HitTarget = Payload.Target;
	if (nullptr == HitTarget) return;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitTarget);

	if (nullptr == SourceASC || nullptr == TargetASC) return;

	// TODO : 팀필터 거르기

	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(EffectClass, GetAbilityLevel());
	if (false == Spec.IsValid()) return;

	float AttackPower = SourceASC->GetNumericAttribute(UDC1PlayerSet::GetAttackPowerAttribute());
	Spec.Data->SetSetByCallerMagnitude(DC1PlayerTags::SetByCaller_Damage, -AttackPower);
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
