// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalSkill.h"

#include "AbilitySystemComponent.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Controller/DC1PalAIController.h"
#include "Data/DA/Pal/DC1PalData.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimMontage.h"

UDC1GameplayAbility_PalSkill::UDC1GameplayAbility_PalSkill(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UDC1GameplayAbility_PalSkill::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (false == CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const ADC1PalCharacter* Pal = (nullptr != ActorInfo) ? Cast<ADC1PalCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	const UDC1PalData* PalData = (nullptr != Pal) ? Pal->GetPalData() : nullptr;
	if (nullptr == PalData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: No PalData on avatar."), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CacheSkillDef();

	// 표가 몽타주 슬롯을 지정하면 그걸 쓴다. 실제 몽타주는 종별 PalData 가 푼다
	const FGameplayTag& ResolvedMontageTag = (true == bHasSkillDef && true == SkillDef.MontageTag.IsValid())
		? SkillDef.MontageTag
		: MontageTag;

	// 스폰 시점에 미리 로드하는 쪽으로 바꿔야됨
	UAnimMontage* Montage = PalData->FindMontageByTag(ResolvedMontageTag).LoadSynchronous();
	if (nullptr == Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: No montage for tag [%s]. Check PalData.Montages."), *GetNameSafe(Pal), *ResolvedMontageTag.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (true == TriggerEventTag.IsValid())
	{
		UAbilityTask_WaitGameplayEvent* TriggerTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TriggerEventTag, nullptr, false, true);
		if (nullptr != TriggerTask)
		{
			TriggerTask->EventReceived.AddDynamic(this, &ThisClass::OnTriggerEventReceived);
			TriggerTask->ReadyForActivation();
		}
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage);
	if (nullptr == MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageAborted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageAborted);
	MontageTask->ReadyForActivation();
}

void UDC1GameplayAbility_PalSkill::CacheSkillDef()
{
	bHasSkillDef = false;

	const AActor* Avatar = GetAvatarActorFromActorInfo();
	const APawn* Pal = Cast<APawn>(Avatar);
	const ADC1PalAIController* AIController = (nullptr != Pal) ? Cast<ADC1PalAIController>(Pal->GetController()) : nullptr;
	if (nullptr == AIController)
	{
		return;
	}

	// 컨트롤러가 이번에 고른 줄. 표에 없는 스킬이면 널이고, 그때는 이 BP 값을 쓴다
	if (const FDC1PalSkillDef* Found = AIController->GetSelectedSkillDef())
	{
		SkillDef = *Found;
		bHasSkillDef = true;
	}
}

void UDC1GameplayAbility_PalSkill::OnTriggerEventReceived(FGameplayEventData Payload)
{
	if (nullptr == CurrentActorInfo || false == CurrentActorInfo->IsNetAuthority())
	{
		return;
	}

	PerformSkillEffect();
}

void UDC1GameplayAbility_PalSkill::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDC1GameplayAbility_PalSkill::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
