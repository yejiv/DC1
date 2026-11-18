// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Effects/DC1GameplayEffect_PalPassive.h"

#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystem/Attribute/DC1CharacterAttributeSet.h"
#include "Tags/Effect/DC1GameplayTag_Effect.h"

UDC1GameplayEffect_PalPassive::UDC1GameplayEffect_PalPassive(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	auto AddMultiplier = [this](const FGameplayAttribute& InAttribute, const FGameplayTag& InDataTag)
		{
			FSetByCallerFloat SetByCaller;
			SetByCaller.DataTag = InDataTag;

			FGameplayModifierInfo Modifier;
			Modifier.Attribute = InAttribute;
			Modifier.ModifierOp = EGameplayModOp::MultiplyCompound;
			Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);

			Modifiers.Add(Modifier);
		};

	AddMultiplier(UDC1CharacterAttributeSet::GetAttackPowerAttribute(), DC1EffectTags::SetByCaller_PalPassive_Attack);
	AddMultiplier(UDC1AttributeSet::GetDefensePowerAttribute(), DC1EffectTags::SetByCaller_PalPassive_Defense);
	AddMultiplier(UDC1CharacterAttributeSet::GetWorkSpeedAttribute(), DC1EffectTags::SetByCaller_PalPassive_WorkSpeed);
}
