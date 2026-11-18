// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/DC1GA_Player_Downed.h"

#include "Character/Player/DC1Player.h"

#include "Tags/DC1GamePlayTags.h"
#include "AbilitySystem/Attribute/DC1PlayerSet.h"

#include "AbilitySystemComponent.h"

UDC1GA_Player_Downed::UDC1GA_Player_Downed(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	ActivationOwnedTags.AddTag(DC1StateTags::State_Downed);

	ActivationBlockedTags.RemoveTag(DC1StateTags::State_Downed);
}

void UDC1GA_Player_Downed::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 게이지 / 감소 GE는 서버만 ( 결과는 Attribute 복제로 전달 )
	if (false == HasAuthority(&ActivationInfo))
		return;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (nullptr == ASC)
		return;

	// 기절 게이지 가득
	const float MaxGauge = ASC->GetNumericAttribute(UDC1PlayerSet::GetMaxDownedGaugeAttribute());
	ASC->SetNumericAttributeBase(UDC1PlayerSet::GetDownedGaugeAttribute(), MaxGauge);

	// 시간 감소 시작
	if (nullptr != DrainEffectClass)
	{
		const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DrainEffectClass, GetAbilityLevel());
		if (true == SpecHandle.IsValid())
		{
			DrainEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
	}
}

void UDC1GA_Player_Downed::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 시간 감소 정지 ( 살아남 / 사망 )
	if (true == DrainEffectHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveActiveGameplayEffect(DrainEffectHandle);
		}
		DrainEffectHandle.Invalidate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

ADC1Player* UDC1GA_Player_Downed::GetPlayer() const
{
	return Cast<ADC1Player>(GetAvatarActorFromActorInfo());
}
