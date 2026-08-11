// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Effects/DC1GameplayEffect_PalDamage.h"

#include "Character/Pal/DC1PalAttributeSet.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

UDC1GameplayEffect_PalDamage::UDC1GameplayEffect_PalDamage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = DC1PalTags::SetByCaller_Damage;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UDC1PalAttributeSet::GetDamageAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);

	Modifiers.Add(Modifier);
}
