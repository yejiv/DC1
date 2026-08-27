// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "DC1GameplayAbility_PalSkill.generated.h"

struct FGameplayEventData;

/**
 *
 * 몽타주 재생 -> AnimNotify가 보낸 이벤트 -> PerformSkillEffect() 
 * 
 */
UCLASS(Abstract)
class DC1_API UDC1GameplayAbility_PalSkill : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	UDC1GameplayAbility_PalSkill(const FObjectInitializer& ObjectInitializer);

	float					GetMaxRange() const { return MaxRange; }
	float					GetApproachRange() const { return ApproachRange; }

protected:
	virtual void			ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void			PerformSkillEffect() PURE_VIRTUAL(UDC1GameplayAbility_PalSkill::PerformSkillEffect, );

	UFUNCTION()
	void					OnMontageFinished();

	UFUNCTION()
	void					OnMontageAborted();

	UFUNCTION()
	void					OnTriggerEventReceived(FGameplayEventData Payload);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Skill", meta = (Categories = "Anim.Pal"))
	FGameplayTag			MontageTag;

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Skill", meta = (Categories = "Event"))
	FGameplayTag			TriggerEventTag;

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Skill", meta = (ClampMin = "0.0"))
	float					MaxRange = { 150.f };		// 공격이 닿는 한계

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Skill", meta = (ClampMin = "0.0"))
	float					ApproachRange = { 150.f };	// 접근해서 멈출 자리
};
