// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attribute/DC1PalAttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "GenericTeamAgentInterface.h"

UDC1PalAttributeSet::UDC1PalAttributeSet()
{
	// PalData 적용 전 한 프레임용 자리값. ADC1PalCharacter::ApplyPalData 가 곧바로 덮어쓴다.
	// 실제 수치는 DA_PalRegistry 의 StatGrowth + DA_Pal_* 의 종 스탯이 정하므로
	// 여기 값을 레지스트리와 맞추려 하지 않는다 — 맞춰두면 레지스트리를 바꿀 때 따라오지 않아 거짓말이 된다.
	// 방어력만 0 이 아니어야 한다 (데미지 계산의 분모)
	InitHealth(1.f);
	InitMaxHealth(1.f);
	InitAttackPower(1.f);
	InitDefensePower(1.f);
	InitSanity(100.f);
	InitMaxSanity(100.f);
}

void UDC1PalAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PalAttributeSet, Sanity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDC1PalAttributeSet, MaxSanity, COND_None, REPNOTIFY_Always);
}

void UDC1PalAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (GetSanityAttribute() == Attribute)
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxSanity());
	}
}

bool UDC1PalAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (false == Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	if (GetDamageAttribute() != Data.EvaluatedData.Attribute)
	{
		return true;
	}

	const AActor* Causer = Data.EffectSpec.GetContext().GetEffectCauser();
	if (nullptr == Causer)
	{
		return true;
	}

	const IGenericTeamAgentInterface* Self = Cast<const IGenericTeamAgentInterface>(Data.Target.GetAvatarActor());
	if (nullptr == Self)
	{
		return true;
	}

	// 적이 가한 게 아니면 데미지가 아예 들어가지 않는다.
	//
	// 방어력·속성 경감은 여기가 아니라 UDC1PalDamageExecution 에 있다.
	// 이 검사만 여기 남는 이유는 출처를 가리지 않아야 하기 때문이다 —
	// 플레이어가 자기 파트너를 때리는 것도 막아야 하는데, 그쪽은 아직 공용 GE(계산기 없음)를 쓴다
	return (ETeamAttitude::Hostile == Self->GetTeamAttitudeTowards(*Causer));
}

void UDC1PalAttributeSet::OnRep_Sanity(const FGameplayAttributeData& OldSanity)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1PalAttributeSet, Sanity, OldSanity)
}

void UDC1PalAttributeSet::OnRep_MaxSanity(const FGameplayAttributeData& OldMaxSanity)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDC1PalAttributeSet, MaxSanity, OldMaxSanity)
}
