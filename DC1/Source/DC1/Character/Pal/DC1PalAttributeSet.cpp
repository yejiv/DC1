// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Pal/DC1PalAttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UDC1PalAttributeSet::UDC1PalAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
}

void UDC1PalAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PalAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PalAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UDC1PalAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (GetHealthAttribute() == Attribute)
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
}

void UDC1PalAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (GetHealthAttribute() == Data.EvaluatedData.Attribute)
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
}

void UDC1PalAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1PalAttributeSet, Health, OldHealth);
}

void UDC1PalAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1PalAttributeSet, MaxHealth, OldMaxHealth);
}
