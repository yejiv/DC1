// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attribute/DC1PalAttributeSet.h"

#include "GameplayEffectExtension.h"

UDC1PalAttributeSet::UDC1PalAttributeSet()
{
	// PalData가 적용되기 전까지의 안전값. ADC1PalCharacter::ApplyPalData에서 덮어씀
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitAttackPower(10.f);
}

void UDC1PalAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (GetDamageAttribute() != Data.EvaluatedData.Attribute)
	{
		return;
	}

	// 메타 어트리뷰트는 즉시 비운다. 남겨두면 다음 GE에 누적된다.
	const float LocalDamage = GetDamage();
	SetDamage(0.f);

	if (LocalDamage <= 0.f)
	{
		return;
	}

	SetHealth(FMath::Clamp(GetHealth() - LocalDamage, 0.f, GetMaxHealth()));

	UE_LOG(LogTemp, Log, TEXT("[Pal] %s took %.1f damage from %s. Health = %.1f / %.1f"),
		*GetNameSafe(Data.Target.GetAvatarActor()),
		LocalDamage,
		*GetNameSafe(Data.EffectSpec.GetContext().GetEffectCauser()),
		GetHealth(), GetMaxHealth());

	// Health <= 0 이면 여기서 사망 처리
}
