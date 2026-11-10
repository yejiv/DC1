// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Effects/DC1GameplayEffect_EquipStat.h"
#include "AbilitySystem/Attribute/DC1PlayerSet.h"
#include "Tags/Effect/DC1GameplayTag_Effect.h"

UDC1GameplayEffect_EquipStat::UDC1GameplayEffect_EquipStat(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	auto AddMultiplier = [this](const FGameplayAttribute& InAttribute, const FGameplayTag& InDataTag)
		{
			FSetByCallerFloat SetByCaller;
			SetByCaller.DataTag = InDataTag;

			FGameplayModifierInfo Modifier;
			Modifier.Attribute = InAttribute;
			Modifier.ModifierOp = EGameplayModOp::Additive;
			Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);

			Modifiers.Add(Modifier);
		};

	AddMultiplier(UDC1AttributeSet::GetDefensePowerAttribute(), DC1EffectTags::SetByCaller_Equip_Defense);
	AddMultiplier(UDC1AttributeSet::GetMaxHealthAttribute(), DC1EffectTags::SetByCaller_Equip_Health);
	AddMultiplier(UDC1PlayerSet::GetMaxShieldAttribute(), DC1EffectTags::SetByCaller_Equip_Shield);
}
