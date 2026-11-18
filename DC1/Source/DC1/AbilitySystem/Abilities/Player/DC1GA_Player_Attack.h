// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "DC1GA_Player_Attack.generated.h"

class ADC1Weapon;
class UGameplayEffect;

/**
 * 플레이어 공격 어빌리티의 공통 베이스.
 * 실제 공격 흐름은 자식(MeleeAttack / RangedAttack)이 구현하고,
 * 여기서는 무기 조회와 데미지 Spec 생성만 담당한다.
 */
UCLASS(Abstract)
class DC1_API UDC1GA_Player_Attack : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	UDC1GA_Player_Attack(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void		EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 현재 장착 중인 무기. 없으면 nullptr
	ADC1Weapon*					GetEquippedWeapon() const;

	// 공격력 x 무기 배율로 데미지 Spec을 만든다. 실패하면 무효 핸들
	FGameplayEffectSpecHandle	MakeDamageSpec() const;

	// 즉시 판정용. 투사체처럼 나중에 맞는 경우는 Spec을 직접 들고 가야 한다
	void								ApplyDamage(const AActor* Target) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Effect")
	TSubclassOf<UGameplayEffect>	EffectClass;
};
