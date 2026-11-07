// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/DC1GA_Player_Revive.h"

#include "Character/Player/DC1Player.h"
#include "Character/Player/DC1PlayerController.h"
#include "Component/Interact/DC1InteractComponent.h"

#include "Component/DC1AbilitySystemComponent.h"

#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

UDC1GA_Player_Revive::UDC1GA_Player_Revive(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UDC1GA_Player_Revive::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (false == Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
		return false;

	// 서버는 아직 Target을 모르기때문에 true 반환
	if (nullptr == ActorInfo || false == ActorInfo->IsLocallyControlled())
		return true;

	const ADC1Player* Target = FindReviveTarget(ActorInfo);
	return (nullptr != Target) ? Target->CanBeRevived() : false;
}

void UDC1GA_Player_Revive::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 키 입력 취소 Task
	UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	if (nullptr != ReleaseTask)
	{
		ReleaseTask->OnRelease.AddDynamic(this, &ThisClass::HandleInputReleased);
		ReleaseTask->ReadyForActivation();
	}

	// 서버 내에선 Target이 지정될 때까지 기다리다가 Target 오면 BeginRevive 호출
	if (false == ActorInfo->IsLocallyControlled())
	{
		WaitReviveTarget();
		return;
	}

	ADC1Player* Target = FindReviveTarget(ActorInfo);
	if (nullptr == Target)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 리슨 서버 호스트
	if (true == ActorInfo->IsNetAuthority())
	{
		BeginRevive(Target);
	}
	// 원격 클라(치료 주체)
	// 클라 -> 서버로 Target 전달
	else
	{
		SendReviveTarget();
	}
}

void UDC1GA_Player_Revive::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (true == ReviveTarget.IsValid())
	{
		ReviveTarget->StopBeingRevived();
	}
	ReviveTarget.Reset();

	if (true == TargetDataSetHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->AbilityTargetDataSetDelegate(Handle, ActivationInfo.GetActivationPredictionKey()).Remove(TargetDataSetHandle);
		}
	}
	TargetDataSetHandle.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDC1GA_Player_Revive::HandleInputReleased(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UDC1GA_Player_Revive::HandleReviveDelayFinished()
{
	const ADC1PlayerController* PC = Cast<ADC1PlayerController>(CurrentActorInfo->PlayerController.Get());
	const UDC1InteractComponent* InteractComponent = (nullptr != PC) ? PC->GetInteractComponent() : nullptr;

	// 홀드 중 상대가 사망했거나 사거리를 벗어났을 수 있으므로 완료 시점에 재검증
	const bool bIsValidTarget = true == ReviveTarget.IsValid() && nullptr != InteractComponent &&
		true == ReviveTarget->GetDownState() &&
		true == InteractComponent->IsInInteractRange(Cast<ADC1Player>(CurrentActorInfo->AvatarActor.Get()), ReviveTarget.Get());

	if (true == bIsValidTarget)
	{
		ReviveTarget->CompleteRevived();
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false == bIsValidTarget);
}

void UDC1GA_Player_Revive::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag InActivationTag)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const FPredictionKey PredictionKey = CurrentActivationInfo.GetActivationPredictionKey();

	ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, PredictionKey).Remove(TargetDataSetHandle);
	ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, PredictionKey);
	TargetDataSetHandle.Reset();

	ADC1Player* Target = {};
	if (true == InData.IsValid(0))
	{
		const TArray<TWeakObjectPtr<AActor>> Actors = InData.Get(0)->GetActors();
		if (Actors.Num() > 0)
		{
			Target = Cast<ADC1Player>(Actors[0].Get());
		}
	}

	if (nullptr == Target)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	BeginRevive(Target);
}

void UDC1GA_Player_Revive::WaitReviveTarget()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const FPredictionKey PredictionKey = CurrentActivationInfo.GetActivationPredictionKey();

	TargetDataSetHandle = ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, PredictionKey).AddUObject(this, &ThisClass::OnTargetDataReceived);

	ASC->CallReplicatedTargetDataDelegatesIfSet(CurrentSpecHandle, PredictionKey);
}

void UDC1GA_Player_Revive::BeginRevive(ADC1Player* InTarget)
{
	const ADC1PlayerController* PC = Cast<ADC1PlayerController>(CurrentActorInfo->PlayerController.Get());
	const UDC1InteractComponent* InteractComponent = (nullptr != PC) ? PC->GetInteractComponent() : nullptr;

	if (nullptr == InTarget || nullptr == InteractComponent || false == InTarget->CanBeRevived() ||
		false == InteractComponent->IsInInteractRange(Cast<ADC1Player>(CurrentActorInfo->AvatarActor.Get()), InTarget))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	ReviveTarget = InTarget;

	UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, PC->GetReviveDuration());
	DelayTask->OnFinish.AddDynamic(this, &ThisClass::HandleReviveDelayFinished);
	DelayTask->ReadyForActivation();

	ReviveTarget->StartBeingRevived();
}

void UDC1GA_Player_Revive::SendReviveTarget()
{
	ADC1Player* Target = FindReviveTarget(CurrentActorInfo);
	if (nullptr == Target)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	FScopedPredictionWindow ScopedPrediction(ASC);

	FGameplayAbilityTargetData_ActorArray* ActorData = new FGameplayAbilityTargetData_ActorArray();
	ActorData->TargetActorArray.Add(Target);

	FGameplayAbilityTargetDataHandle DataHandle(ActorData);
	ASC->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(),
		DataHandle, FGameplayTag(), ASC->ScopedPredictionKey);
}

ADC1Player* UDC1GA_Player_Revive::FindReviveTarget(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const ADC1PlayerController* PC = Cast<ADC1PlayerController>(ActorInfo->PlayerController.Get());
	const UDC1InteractComponent* InteractComponent = (nullptr != PC) ? PC->GetInteractComponent() : nullptr;

	return (nullptr != InteractComponent) ? Cast<ADC1Player>(InteractComponent->GetCurrentTarget()) : nullptr;
}
