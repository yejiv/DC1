// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attribute/DC1PalAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "GenericTeamAgentInterface.h"

UDC1PalAttributeSet::UDC1PalAttributeSet()
{
	// PalData가 적용되기 전까지의 안전값. ADC1PalCharacter::ApplyPalData에서 덮어씀
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitAttackPower(10.f);
}

bool UDC1PalAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (false == Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	if (GetDamageAttribute() != Data.EvaluatedData.Attribute)
	{
		return true;
	}

	const AActor* Causer = Data.EffectSpec.GetContext().GetEffectCauser();
	if (nullptr == Causer)
	{
		return true;
	}

	const IGenericTeamAgentInterface* Self = Cast<const IGenericTeamAgentInterface>(Data.Target.GetAvatarActor());
	if (nullptr == Self)
	{
		return true;
	}

	// 적이 가한 게 아니면 데미지가 아예 들어가지 않는다.
	return (ETeamAttitude::Hostile == Self->GetTeamAttitudeTowards(*Causer));
}
