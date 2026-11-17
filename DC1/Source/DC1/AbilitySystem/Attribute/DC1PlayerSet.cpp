// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attribute/DC1PlayerSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Tags/DC1GamePlayTags.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Component/Widget/DC1DamageNumberComponent.h"
#include "Character/Player/DC1Player.h"

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
	// 기절 진입 시 Max로 채운다
	InitDownedGauge(0.f);
	InitMaxDownedGauge(100.f);
}

void UDC1PlayerSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PlayerSet, MaxWeight, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PlayerSet, Satiety, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PlayerSet, MaxSatiety, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PlayerSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PlayerSet, MaxShield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PlayerSet, DownedGauge, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PlayerSet, MaxDownedGauge, COND_None, REPNOTIFY_Always);
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

	if (GetDownedGaugeAttribute() == Attribute)
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxDownedGauge());
	}

	if (GetMaxWeightAttribute() == Attribute || GetMaxShieldAttribute() == Attribute || GetMaxDownedGaugeAttribute() == Attribute)
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

	// 기절 게이지는 주기 GE가 Base를 깎는다
	if (GetDownedGaugeAttribute() == Attribute)
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxDownedGauge());
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

void UDC1PlayerSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	if (GetDamageAttribute() != Data.EvaluatedData.Attribute)
	{
		Super::PostGameplayEffectExecute(Data);
		return;
	}

	if (GetDamage() > 0.f)
	{
		if (UDC1AbilitySystemComponent* DC1ASC = Cast<UDC1AbilitySystemComponent>(&Data.Target))
		{
			DC1ASC->SetExclusiveStateTag(DC1StateTags::State_Shield_RegenBlocked, DC1StateTags::State_Shield_RegenBlocked);
		}
	}

	const bool IsDowned = Data.Target.HasMatchingGameplayTag(DC1StateTags::State_Downed);

	if (false == IsDowned && GetShield() <= 0.f)
	{
		const float OldHealth = GetHealth();
		Super::PostGameplayEffectExecute(Data);

		// 일반 피격음 - 체력이 실제로 깎였을 때만
		ADC1Player* Player = Cast<ADC1Player>(Data.Target.GetAvatarActor());
		if (nullptr != Player && GetHealth() < OldHealth)
		{
			Player->PlayWorldSound(Player->GetHitSound(), Player->GetActorLocation());
		}
		return;
	}

	// 메타 어트리뷰트는 즉시 비운다. 남겨두면 다음 GE에 누적된다.
	const float LocalDamage = GetDamage();
	SetDamage(0.f);

	if (LocalDamage <= 0.f)
	{
		return;
	}

	if (true == IsDowned)
	{
		SetDownedGauge(FMath::Clamp(GetDownedGauge() - LocalDamage, 0.f, GetMaxDownedGauge()));
		return;
	}

	// 데미지 Notify -> World UI 표시
	const float OldShield = GetShield();
	SetShield(FMath::Max(GetShield() - LocalDamage, 0.f));
	UDC1DamageNumberComponent::NotifyDamage(Data, OldShield - GetShield(), EDC1DamageNumberType::Shield);

	// 보호막이 깨지는 순간은 파괴음, 남아 있으면 보호막 피격음
	if (ADC1Player* Player = Cast<ADC1Player>(Data.Target.GetAvatarActor()))
	{
		USoundBase* ShieldSound = (GetShield() <= 0.f) ? Player->GetShieldBreakSound() : Player->GetShieldHitSound();
		Player->PlayWorldSound(ShieldSound, Player->GetActorLocation());
	}
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

void UDC1PlayerSet::OnRep_DownedGauge(const FGameplayAttributeData& OldDownedGauge)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1PlayerSet, DownedGauge, OldDownedGauge);
}

void UDC1PlayerSet::OnRep_MaxDownedGauge(const FGameplayAttributeData& OldMaxDownedGauge)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1PlayerSet, MaxDownedGauge, OldMaxDownedGauge);
}
