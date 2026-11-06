// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalSleep.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimMontage.h"

UDC1GameplayAbility_PalSleep::UDC1GameplayAbility_PalSleep(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	ActivationOwnedTags.AddTag(DC1PalTags::State_Pal_Sleeping);
}

void UDC1GameplayAbility_PalSleep::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (false == CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bWakeRequested = false;

	UAbilityTask_WaitGameplayEvent* WakeTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, DC1PalTags::Event_Pal_Wake);
	if (nullptr != WakeTask)
	{
		WakeTask->EventReceived.AddDynamic(this, &ThisClass::OnWake);
		WakeTask->ReadyForActivation();
	}

	PlaySleepMontage(DC1PalTags::Anim_Pal_Sleep_Start);
}

void UDC1GameplayAbility_PalSleep::PlaySleepMontage(const FGameplayTag& InMontageTag)
{
	// 앞 몽타주 태스크를 먼저 끝냄. (남겨 두면 다음 몽타주에 끊긴 알림이 와서 어빌리티가 취소)
	if (nullptr != MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(GetAvatarActorFromActorInfo());
	const UDC1PalData* PalData = (nullptr != Pal) ? Pal->GetPalData() : nullptr;
	UAnimMontage* Montage = (nullptr != PalData) ? PalData->FindMontageByTag(InMontageTag).LoadSynchronous() : nullptr;
	if (nullptr == Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: No montage for tag [%s]. Check PalData.Montages."), *GetNameSafe(Pal), *InMontageTag.ToString());
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	CurrentMontageTag = InMontageTag;

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage);
	if (nullptr == MontageTask)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageAborted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageAborted);
	MontageTask->ReadyForActivation();
}

void UDC1GameplayAbility_PalSleep::OnWake(FGameplayEventData InPayload)
{
	// 지금 몽타주가 끝나면 End
	bWakeRequested = true;
}

void UDC1GameplayAbility_PalSleep::OnMontageFinished()
{
	if (true == CurrentMontageTag.MatchesTagExact(DC1PalTags::Anim_Pal_Sleep_End))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// Start 나 Loop 가 끝남.
	PlaySleepMontage(bWakeRequested ? DC1PalTags::Anim_Pal_Sleep_End : DC1PalTags::Anim_Pal_Sleep_Loop);
}

void UDC1GameplayAbility_PalSleep::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
