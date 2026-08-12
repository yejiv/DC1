// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/DC1PlayerState.h"

#include "Component/DC1AbilitySystemComponent.h"

ADC1PlayerState::ADC1PlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UDC1AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	SetNetUpdateFrequency(100.f);
}

UAbilitySystemComponent* ADC1PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
