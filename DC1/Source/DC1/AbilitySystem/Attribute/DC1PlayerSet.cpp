// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attribute/DC1PlayerSet.h"

#include "GameplayEffectExtension.h"
#include "Tags/DC1GamePlayTags.h"

UDC1PlayerSet::UDC1PlayerSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitStamina(100.f);
	InitMaxStamina(100.f);
	InitAttackPower(10.f);
	InitLevel(3.f);
	InitWorkSpeed(10.f);
}

bool UDC1PlayerSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (false == Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	if (GetDamageAttribute() != Data.EvaluatedData.Attribute)
	{
		return true;
	}

	if (true == Data.Target.HasMatchingGameplayTag(DC1StateTags::State_Invincible))
	{
		return false;
	}

	return true;
}