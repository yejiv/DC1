// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attribute/DC1PalAttributeSet.h"

#include "GameplayEffectExtension.h"

UDC1PalAttributeSet::UDC1PalAttributeSet()
	: Super()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
}

void UDC1PalAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UDC1PalAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}
