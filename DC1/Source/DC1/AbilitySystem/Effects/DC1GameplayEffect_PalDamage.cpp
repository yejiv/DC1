// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Effects/DC1GameplayEffect_PalDamage.h"

#include "AbilitySystem/Executions/DC1PalDamageExecution.h"

UDC1GameplayEffect_PalDamage::UDC1GameplayEffect_PalDamage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Modifier 가 없다. 데미지 값을 Execution 이 만들어 Damage 메타 어트리뷰트에 넣는다
	FGameplayEffectExecutionDefinition ExecutionDefinition;
	ExecutionDefinition.CalculationClass = UDC1PalDamageExecution::StaticClass();

	Executions.Add(ExecutionDefinition);
}
