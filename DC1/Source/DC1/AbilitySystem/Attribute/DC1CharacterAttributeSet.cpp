// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attribute/DC1CharacterAttributeSet.h"

#include "Net/UnrealNetwork.h"

UDC1CharacterAttributeSet::UDC1CharacterAttributeSet()
{
	InitMoveSpeed(100.f);
}

void UDC1CharacterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UDC1CharacterAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1CharacterAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1CharacterAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1CharacterAttributeSet, Level, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1CharacterAttributeSet, WorkSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1CharacterAttributeSet, Experience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1CharacterAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
}

void UDC1CharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (GetStaminaAttribute() == Attribute)
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}

	if (GetWorkSpeedAttribute() == Attribute || GetMoveSpeedAttribute() == Attribute)
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

void UDC1CharacterAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	// PreAttributeChange는 CurrentValue만 막는다. 즉시/주기 GE는 BaseValue를 직접 올리므로
	// 여기서 막지 않으면 표시값만 Max에 붙어 있고 실제 base가 무한정 쌓인다
	if (GetStaminaAttribute() == Attribute)
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());
	}
}

void UDC1CharacterAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1CharacterAttributeSet, Stamina, OldStamina);
}

void UDC1CharacterAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1CharacterAttributeSet, MaxStamina, OldMaxStamina);
}

void UDC1CharacterAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1CharacterAttributeSet, AttackPower, OldAttackPower);
}

void UDC1CharacterAttributeSet::OnRep_Level(const FGameplayAttributeData& OldLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1CharacterAttributeSet, Level, OldLevel);
}

void UDC1CharacterAttributeSet::OnRep_Experience(const FGameplayAttributeData& OldExperience)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1CharacterAttributeSet, Experience, OldExperience);
}

void UDC1CharacterAttributeSet::OnRep_WorkSpeed(const FGameplayAttributeData& OldWorkSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1CharacterAttributeSet, WorkSpeed, OldWorkSpeed);
}

void UDC1CharacterAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1CharacterAttributeSet, MoveSpeed, OldMoveSpeed);
}
