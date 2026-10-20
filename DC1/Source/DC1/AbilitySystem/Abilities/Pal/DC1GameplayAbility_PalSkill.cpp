// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalSkill.h"

#include "AbilitySystemComponent.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "AbilitySystem/Effects/DC1GameplayEffect_PalSkillCooldown.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

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

	CacheSkillDef(Handle, ActorInfo);

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

void UDC1GameplayAbility_PalSkill::CacheSkillDef(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo)
{
	bHasSkillDef = false;

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	const FDC1PalSkillDef* Found = (nullptr != Registry) ? Registry->FindSkillDef(FindSkillTag(Handle, ActorInfo)) : nullptr;
	if (nullptr == Found)
	{
		return;
	}

	SkillDef = *Found;
	bHasSkillDef = true;
}

FGameplayTag UDC1GameplayAbility_PalSkill::FindSkillTag(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	const UAbilitySystemComponent* ASC = (nullptr != ActorInfo) ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const FGameplayAbilitySpec* Spec = (nullptr != ASC) ? ASC->FindAbilitySpecFromHandle(Handle) : nullptr;
	if (nullptr == Spec)
	{
		return FGameplayTag::EmptyTag;
	}

	static const FGameplayTag SkillRoot = FGameplayTag::RequestGameplayTag(TEXT("Pal.Skill"));

	for (const FGameplayTag& Tag : Spec->GetDynamicSpecSourceTags())
	{
		if (true == Tag.MatchesTag(SkillRoot))
		{
			return Tag;
		}
	}

	return FGameplayTag::EmptyTag;
}

bool UDC1GameplayAbility_PalSkill::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (false == Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	const FGameplayTag CooldownTag = DC1PalTags::FindSkillCooldownTag(FindSkillTag(Handle, ActorInfo));
	const UAbilitySystemComponent* ASC = (nullptr != ActorInfo) ? ActorInfo->AbilitySystemComponent.Get() : nullptr;

	return (false == CooldownTag.IsValid() || nullptr == ASC || false == ASC->HasMatchingGameplayTag(CooldownTag));
}

void UDC1GameplayAbility_PalSkill::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const FGameplayTag SkillTag = FindSkillTag(Handle, ActorInfo);
	const FGameplayTag CooldownTag = DC1PalTags::FindSkillCooldownTag(SkillTag);

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	const FDC1PalSkillDef* Def = (nullptr != Registry) ? Registry->FindSkillDef(SkillTag) : nullptr;
	if (nullptr == Def || Def->Cooldown <= 0.f)
	{
		return;
	}

	if (false == CooldownTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: No cooldown tag for [%s]. Add Cooldown.%s in DC1GameplayTag_Pal.cpp."),
			*GetName(), *SkillTag.ToString(), *SkillTag.ToString());
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, UDC1GameplayEffect_PalSkillCooldown::StaticClass(), GetAbilityLevel(Handle, ActorInfo));
	if (false == SpecHandle.IsValid())
	{
		return;
	}

	SpecHandle.Data->SetDuration(Def->Cooldown, true);
	SpecHandle.Data->DynamicGrantedTags.AddTag(CooldownTag);

	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
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
