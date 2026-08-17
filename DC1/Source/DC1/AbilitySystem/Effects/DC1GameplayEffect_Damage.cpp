// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Effects/DC1GameplayEffect_Damage.h"

#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "Tags/Effect/DC1GameplayTag_Effect.h"

UDC1GameplayEffect_Damage::UDC1GameplayEffect_Damage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = DC1EffectTags::SetByCaller_Damage;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UDC1AttributeSet::GetDamageAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);

	Modifiers.Add(Modifier);
}
