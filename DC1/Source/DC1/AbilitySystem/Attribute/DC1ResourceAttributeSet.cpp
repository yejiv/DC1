// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attribute/DC1ResourceAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Interface/DC1WorkTargetInterface.h"

bool UDC1ResourceAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (false == Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	if (GetDamageAttribute() != Data.EvaluatedData.Attribute)
	{
		return true;
	}

	// WorkTarget Cast ( WorkType 확인 위함 )
	const IDC1WorkTargetInterface* WorkTarget = Cast<IDC1WorkTargetInterface>(GetOwningActor());
	if (nullptr == WorkTarget)
	{
		return true;
	}

	const FGameplayTag WorkTypeTag = WorkTarget->GetWorkTypeTag();
	if (false == WorkTypeTag.IsValid())
	{
		return true;
	}
	// GA_Player_Attack에서 WorkTypeTag로 배율 전달
	Data.EvaluatedData.Magnitude *= Data.EffectSpec.GetSetByCallerMagnitude(WorkTypeTag, false, 1.f);

	return true;
}
