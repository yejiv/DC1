// Fill out your copyright notice in the Description page of Project Settings.


#include "ViewModel/Player/DC1PlayerStatusViewModel.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/DC1PlayerSet.h"
#include "Character/Player/DC1PlayerState.h"

void UDC1PlayerStatusViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	ClearSource();

	if (nullptr == InPlayerState)
		return;

	AbilitySystemComponent = InPlayerState->GetAbilitySystemComponent();

	if (nullptr == AbilitySystemComponent)
		return;

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetHealthAttribute()).AddUObject(this, &UDC1PlayerStatusViewModel::HandleHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetMaxHealthAttribute()).AddUObject(this, &UDC1PlayerStatusViewModel::HandleMaxHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetStaminaAttribute()).AddUObject(this, &UDC1PlayerStatusViewModel::HandleStaminaChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetMaxStaminaAttribute()).AddUObject(this, &UDC1PlayerStatusViewModel::HandleMaxStaminaChanged);

	BroadcastAll();
}

void UDC1PlayerStatusViewModel::ClearSource()
{
	if (false == AbilitySystemComponent.IsValid())
		return;

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetHealthAttribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetMaxHealthAttribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetStaminaAttribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetMaxStaminaAttribute()).RemoveAll(this);

	AbilitySystemComponent.Reset();
}

float UDC1PlayerStatusViewModel::GetHealth() const
{
	return GetAttributeValue(UDC1PlayerSet::GetHealthAttribute());
}

float UDC1PlayerStatusViewModel::GetMaxHealth() const
{
	return GetAttributeValue(UDC1PlayerSet::GetMaxHealthAttribute());
}

float UDC1PlayerStatusViewModel::GetHealthPercent() const
{
	float MaxHealth = GetMaxHealth();
	if (MaxHealth <= 0.f)
	{
		return 0.f;
	}

	return GetHealth() / MaxHealth;
}

float UDC1PlayerStatusViewModel::GetStamina() const
{
	return GetAttributeValue(UDC1PlayerSet::GetStaminaAttribute());
}

float UDC1PlayerStatusViewModel::GetMaxStamina() const
{
	return GetAttributeValue(UDC1PlayerSet::GetMaxStaminaAttribute());
}

float UDC1PlayerStatusViewModel::GetStaminaPercent() const
{
	float MaxStamina = GetMaxStamina();
	if (MaxStamina <= 0.f)
	{
		return 0.f;
	}

	return GetStamina() / MaxStamina;
}

void UDC1PlayerStatusViewModel::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	OnHealthChanged.Broadcast(GetHealth(), GetMaxHealth());
}

void UDC1PlayerStatusViewModel::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	OnHealthChanged.Broadcast(GetHealth(), GetMaxHealth());
}

void UDC1PlayerStatusViewModel::HandleStaminaChanged(const FOnAttributeChangeData& Data)
{
	OnStaminaChanged.Broadcast(GetStamina(), GetMaxStamina());
}

void UDC1PlayerStatusViewModel::HandleMaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	OnStaminaChanged.Broadcast(GetStamina(), GetMaxStamina());
}

void UDC1PlayerStatusViewModel::BroadcastAll()
{
	OnHealthChanged.Broadcast(GetHealth(), GetMaxHealth());
	OnStaminaChanged.Broadcast(GetStamina(), GetMaxStamina());
}

float UDC1PlayerStatusViewModel::GetAttributeValue(const FGameplayAttribute& InAttribute) const
{
	if (false == AbilitySystemComponent.IsValid())
		return 0.f;

	return AbilitySystemComponent->GetNumericAttribute(InAttribute);
}