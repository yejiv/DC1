#include "AbilitySystem/Abilities/Player/DC1GA_Player_Eat.h"

#include "AbilitySystem/Effects/DC1GameplayEffect_SatietyRestore.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Data/DA/Anim/DC1AnimData.h"
#include "System/DC1AssetManager.h"
#include "Tags/DC1GamePlayTags.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UDC1GA_Player_Eat::UDC1GA_Player_Eat(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	FAbilityTriggerData Trigger = {};
	Trigger.TriggerTag = DC1PlayerTags::Event_Player_Eat;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);

	ActivationBlockedTags.AddTag(DC1StateTags::State_Climbing);
	ActivationBlockedTags.AddTag(DC1StateTags::State_Gliding);
	ActivationBlockedTags.AddTag(DC1StateTags::State_Dodging);
	ActivationBlockedTags.AddTag(DC1StateTags::State_Working);
	ActivationBlockedTags.AddTag(DC1StateTags::State_Aiming);
	ActivationBlockedTags.AddTag(DC1StateTags::State_Build_Placement);
}

bool UDC1GA_Player_Eat::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (false == Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 현재 다른 액션 중(Montage)이면 음식 섭취 불가 ( ActivationBlockedTags + 공격 등 State 부여 안하는 GA 확인 )
	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (nullptr == ASC || nullptr != ASC->GetCurrentMontage())
	{
		return false;
	}

	return true;
}

void UDC1GA_Player_Eat::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (nullptr == TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 포만감 GE는 서버만
	if (true == HasAuthority(&ActivationInfo))
	{
		ApplyFoodEffects(TriggerEventData->EventMagnitude);
	}

	const UDC1AnimData* AnimData = UDC1AssetManager::GetAssetByTag<UDC1AnimData>(DC1AssetTags::Asset_Anim_Player);
	UAnimMontage* Montage = (nullptr != AnimData) ? AnimData->FindMontageByTag(DC1AnimTags::Anim_Action_Eat) : nullptr;
	if (nullptr == Montage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, 1.f);
	if (nullptr == PlayMontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
	PlayMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
	PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
	PlayMontageTask->ReadyForActivation();
}

void UDC1GA_Player_Eat::ApplyFoodEffects(float InSatietyAmount)
{
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(UDC1GameplayEffect_SatietyRestore::StaticClass());
	if (false == SpecHandle.IsValid())
	{
		return;
	}

	SpecHandle.Data->SetSetByCallerMagnitude(DC1EffectTags::SetByCaller_Satiety, InSatietyAmount);
	ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);

	if (UDC1AbilitySystemComponent* DC1ASC = Cast<UDC1AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
	{
		DC1ASC->SetExclusiveStateTag(DC1StateTags::State_Satiety_Full, DC1StateTags::State_Satiety_Full);
	}
}

void UDC1GA_Player_Eat::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
