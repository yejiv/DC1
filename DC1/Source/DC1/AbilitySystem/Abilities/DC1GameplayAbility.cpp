// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/DC1GameplayAbility.h"

#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "Tags/DC1GamePlayTags.h"

UDC1GameplayAbility::UDC1GameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UDC1GameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UDC1GameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UDC1GameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UDC1GameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (false == Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	if (StaminaCost <= 0.f)
	{
		return true;
	}

	// Super는 코스트 GE 클래스만으로 스펙을 만들어 계산한다. 거기엔 SetByCaller 값이 없어서
	// 소모량이 0으로 취급되고 검사가 항상 통과한다. 그래서 잔량은 여기서 직접 본다
	const UAbilitySystemComponent* ASC = (nullptr != ActorInfo) ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (nullptr == ASC)
	{
		return false;
	}

	if (ASC->GetNumericAttribute(UDC1AttributeSet::GetStaminaAttribute()) >= StaminaCost)
	{
		return true;
	}

	// 실패 이유를 엔진 기본 경로와 같은 태그로 알린다
	const FGameplayTag& CostTag = UAbilitySystemGlobals::Get().ActivateFailCostTag;
	if (nullptr != OptionalRelevantTags && true == CostTag.IsValid())
	{
		OptionalRelevantTags->AddTag(CostTag);
	}

	return false;
}

void UDC1GameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const UGameplayEffect* CostGE = GetCostGameplayEffect();

	if (StaminaCost <= 0.f || nullptr == CostGE)
	{
		Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
		return;
	}

	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, CostGE->GetClass(), GetAbilityLevel(Handle, ActorInfo));
	if (false == Spec.IsValid())
	{
		return;
	}

	// 어트리뷰트를 Add로 깎으므로 음수로 넘긴다
	Spec.Data->SetSetByCallerMagnitude(DC1EffectTags::SetByCaller_StaminaCost, -StaminaCost);

	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
}
