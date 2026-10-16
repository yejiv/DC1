// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attribute/DC1PlayerSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Tags/DC1GamePlayTags.h"

UDC1PlayerSet::UDC1PlayerSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitStamina(100.f);
	InitMaxStamina(100.f);
	InitAttackPower(10.f);
	// 0이면 팰 데미지 계산이 DefenseFlatBase(50)로 대체하므로 같은 값으로 시작
	InitDefensePower(50.f);
	InitLevel(3.f);
	InitWorkSpeed(10.f);
	InitMaxWeight(100.f);
	InitSatiety(100.f);
	InitMaxSatiety(100.f);
	InitShield(0.f);
	InitMaxShield(0.f);
}

void UDC1PlayerSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PlayerSet, MaxWeight, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PlayerSet, Satiety, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PlayerSet, MaxSatiety, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PlayerSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PlayerSet, MaxShield, COND_None, REPNOTIFY_Always);
}

void UDC1PlayerSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (GetSatietyAttribute() == Attribute)
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxSatiety());
	}

	if (GetShieldAttribute() == Attribute)
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxShield());
	}

	if (GetMaxWeightAttribute() == Attribute || GetMaxShieldAttribute() == Attribute)
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

void UDC1PlayerSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	// 포만도는 주기 GE로 깎일 예정이라 Stamina처럼 Base도 막는다
	if (GetSatietyAttribute() == Attribute)
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxSatiety());
	}

	// 보호막도 주기 회복 GE가 Base를 올릴 예정
	if (GetShieldAttribute() == Attribute)
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxShield());
	}
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

void UDC1PlayerSet::OnRep_MaxWeight(const FGameplayAttributeData& OldMaxWeight)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1PlayerSet, MaxWeight, OldMaxWeight);
}

void UDC1PlayerSet::OnRep_Satiety(const FGameplayAttributeData& OldSatiety)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1PlayerSet, Satiety, OldSatiety);
}

void UDC1PlayerSet::OnRep_MaxSatiety(const FGameplayAttributeData& OldMaxSatiety)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1PlayerSet, MaxSatiety, OldMaxSatiety);
}

void UDC1PlayerSet::OnRep_Shield(const FGameplayAttributeData& OldShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1PlayerSet, Shield, OldShield);
}

void UDC1PlayerSet::OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1PlayerSet, MaxShield, OldMaxShield);
}
