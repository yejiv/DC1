// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attribute/DC1AttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UDC1AttributeSet::UDC1AttributeSet()
{
}

void UDC1AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UDC1AttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1AttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1AttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1AttributeSet, DefensePower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1AttributeSet, Level, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1AttributeSet, Experience, COND_None, REPNOTIFY_Always);
}

void UDC1AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (GetHealthAttribute() == Attribute)
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
}

void UDC1AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (GetDamageAttribute() == Data.EvaluatedData.Attribute)
	{
		// 메타 어트리뷰트는 즉시 비운다. 남겨두면 다음 GE에 누적된다.
		const float LocalDamage = GetDamage();
		SetDamage(0.f);

		if (LocalDamage <= 0.f)
		{
			return;
		}

		SetHealth(FMath::Clamp(GetHealth() - LocalDamage, 0.f, GetMaxHealth()));

		UE_LOG(LogTemp, Log, TEXT("[DC1] %s took %.1f damage from %s. Health = %.1f / %.1f"),
			*GetNameSafe(Data.Target.GetAvatarActor()),
			LocalDamage,
			*GetNameSafe(Data.EffectSpec.GetContext().GetEffectCauser()),
			GetHealth(), GetMaxHealth());

		return;
	}

	if (GetHealthAttribute() == Data.EvaluatedData.Attribute)
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));

		UE_LOG(LogTemp, Log, TEXT("[DC1] %s Health %+.1f from %s. Health = %.1f / %.1f"),
			*GetNameSafe(Data.Target.GetAvatarActor()),
			Data.EvaluatedData.Magnitude,
			*GetNameSafe(Data.EffectSpec.GetContext().GetEffectCauser()),
			GetHealth(), GetMaxHealth());
	}
}

void UDC1AttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1AttributeSet, Health, OldHealth);
}

void UDC1AttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1AttributeSet, MaxHealth, OldMaxHealth);
}

void UDC1AttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1AttributeSet, AttackPower, OldAttackPower);
}

void UDC1AttributeSet::OnRep_DefensePower(const FGameplayAttributeData& OldDefensePower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1AttributeSet, DefensePower, OldDefensePower);
}

void UDC1AttributeSet::OnRep_Level(const FGameplayAttributeData& OldLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1AttributeSet, Level, OldLevel);
}

void UDC1AttributeSet::OnRep_Experience(const FGameplayAttributeData& OldExperience)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1AttributeSet, Experience, OldExperience);
}
