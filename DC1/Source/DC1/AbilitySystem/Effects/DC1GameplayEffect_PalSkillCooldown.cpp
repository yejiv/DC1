// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Effects/DC1GameplayEffect_PalSkillCooldown.h"

UDC1GameplayEffect_PalSkillCooldown::UDC1GameplayEffect_PalSkillCooldown(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(1.f));
}
