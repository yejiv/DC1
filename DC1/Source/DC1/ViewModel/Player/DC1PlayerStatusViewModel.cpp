// Fill out your copyright notice in the Description page of Project Settings.


#include "ViewModel/Player/DC1PlayerStatusViewModel.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/DC1PlayerSet.h"
#include "Character/Player/DC1PlayerState.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

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
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetSatietyAttribute()).AddUObject(this, &UDC1PlayerStatusViewModel::HandleSatietyChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetMaxSatietyAttribute()).AddUObject(this, &UDC1PlayerStatusViewModel::HandleSatietyChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetShieldAttribute()).AddUObject(this, &UDC1PlayerStatusViewModel::HandleShieldChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetMaxShieldAttribute()).AddUObject(this, &UDC1PlayerStatusViewModel::HandleShieldChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetLevelAttribute()).AddUObject(this, &UDC1PlayerStatusViewModel::HandleLevelChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetExperienceAttribute()).AddUObject(this, &UDC1PlayerStatusViewModel::HandleLevelChanged);

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
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetSatietyAttribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetMaxSatietyAttribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetShieldAttribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetMaxShieldAttribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetLevelAttribute()).RemoveAll(this);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetExperienceAttribute()).RemoveAll(this);

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

float UDC1PlayerStatusViewModel::GetSatiety() const
{
	return GetAttributeValue(UDC1PlayerSet::GetSatietyAttribute());
}

float UDC1PlayerStatusViewModel::GetMaxSatiety() const
{
	return GetAttributeValue(UDC1PlayerSet::GetMaxSatietyAttribute());
}

float UDC1PlayerStatusViewModel::GetShield() const
{
	return GetAttributeValue(UDC1PlayerSet::GetShieldAttribute());
}

float UDC1PlayerStatusViewModel::GetMaxShield() const
{
	return GetAttributeValue(UDC1PlayerSet::GetMaxShieldAttribute());
}

int32 UDC1PlayerStatusViewModel::GetLevel() const
{
	return FMath::TruncToInt32(GetAttributeValue(UDC1PlayerSet::GetLevelAttribute()));
}

int32 UDC1PlayerStatusViewModel::GetRemainingExp() const
{
	const int32 Experience = FMath::TruncToInt32(GetAttributeValue(UDC1PlayerSet::GetExperienceAttribute()));
	return FMath::Max(0, GetExpToNextLevel() - Experience);
}

float UDC1PlayerStatusViewModel::GetExpPercent() const
{
	const int32 ExpToNextLevel = GetExpToNextLevel();
	if (ExpToNextLevel <= 0)
	{
		return 0.f;
	}

	return FMath::Clamp(GetAttributeValue(UDC1PlayerSet::GetExperienceAttribute()) / ExpToNextLevel, 0.f, 1.f);
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

void UDC1PlayerStatusViewModel::HandleSatietyChanged(const FOnAttributeChangeData& Data)
{
	OnSatietyChanged.Broadcast(GetSatiety(), GetMaxSatiety());
}

void UDC1PlayerStatusViewModel::HandleShieldChanged(const FOnAttributeChangeData& Data)
{
	OnShieldChanged.Broadcast(GetShield(), GetMaxShield());
}

void UDC1PlayerStatusViewModel::HandleLevelChanged(const FOnAttributeChangeData& Data)
{
	BroadcastLevel();
}

void UDC1PlayerStatusViewModel::BroadcastLevel()
{
	OnLevelChanged.Broadcast(GetLevel(), GetRemainingExp(), GetExpPercent());
}

void UDC1PlayerStatusViewModel::BroadcastAll()
{
	OnHealthChanged.Broadcast(GetHealth(), GetMaxHealth());
	OnStaminaChanged.Broadcast(GetStamina(), GetMaxStamina());
	OnSatietyChanged.Broadcast(GetSatiety(), GetMaxSatiety());
	OnShieldChanged.Broadcast(GetShield(), GetMaxShield());
	BroadcastLevel();
}

int32 UDC1PlayerStatusViewModel::GetExpToNextLevel() const
{
	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (nullptr == Registry)
		return 0;

	return Registry->GetExpToNextLevel(GetLevel());
}

float UDC1PlayerStatusViewModel::GetAttributeValue(const FGameplayAttribute& InAttribute) const
{
	if (false == AbilitySystemComponent.IsValid())
		return 0.f;

	return AbilitySystemComponent->GetNumericAttribute(InAttribute);
}