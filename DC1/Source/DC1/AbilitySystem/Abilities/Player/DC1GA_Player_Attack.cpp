// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/DC1GA_Player_Attack.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

#include "AbilitySystem/Attribute/DC1PlayerSet.h"
#include "Character/Player/DC1Player.h"
#include "Component/DC1EquipComponent.h"
#include "Item/Equip/Weapon/DC1Weapon.h"
#include "Tags/DC1GamePlayTags.h"

UDC1GA_Player_Attack::UDC1GA_Player_Attack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
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

ADC1Weapon* UDC1GA_Player_Attack::GetEquippedWeapon() const
{
	if (const ADC1Player* Player = Cast<ADC1Player>(GetAvatarActorFromActorInfo()))
	{
		if (nullptr != Player->Equipment)
		{
			return Player->Equipment->CurrentWeapon;
		}
	}

	return nullptr;
}

FGameplayEffectSpecHandle UDC1GA_Player_Attack::MakeDamageSpec() const
{
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (nullptr == SourceASC) return FGameplayEffectSpecHandle();

	FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(EffectClass, GetAbilityLevel());
	if (false == Spec.IsValid()) return Spec;

	const float AttackPower = SourceASC->GetNumericAttribute(UDC1PlayerSet::GetAttackPowerAttribute());

	const ADC1Weapon* Weapon = GetEquippedWeapon();
	const float Multiplier = (nullptr != Weapon) ? Weapon->DamageMultiplier : 1.f;

	// 체력 감산은 AttributeSet의 PostGameplayEffectExecute가 한다. 여기서는 양수로 넘긴다
	Spec.Data->SetSetByCallerMagnitude(DC1EffectTags::SetByCaller_Damage, AttackPower * Multiplier);

	return Spec;
}

void UDC1GA_Player_Attack::ApplyDamage(const AActor* Target) const
{
	if (false == ::IsValid(Target)) return;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	if (nullptr == SourceASC || nullptr == TargetASC) return;

	FGameplayEffectSpecHandle Spec = MakeDamageSpec();
	if (false == Spec.IsValid()) return;

	SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}
