// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalSkill.h"
#include "DC1GameplayAbility_PalNearSkill.generated.h"

/**
 * 근접. 노티파이마다 캡슐 앞 구체로 한 번 판정 (노티파이 한 번에 대상당 1회)
 */
UCLASS()
class DC1_API UDC1GameplayAbility_PalNearSkill : public UDC1GameplayAbility_PalSkill
{
	GENERATED_BODY()

public:
	UDC1GameplayAbility_PalNearSkill(const FObjectInitializer& ObjectInitializer);

	float					GetHitRadius() const { return bHasSkillDef ? SkillDef.HitRadius : HitRadius; }
	float					GetHitReach() const { return bHasSkillDef ? SkillDef.HitReach : HitReach; }

protected:
	virtual void			PerformSkillEffect() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Pal|NearSkill", meta = (ClampMin = "0.0"))
	float					HitRadius = { 80.f };

	UPROPERTY(EditDefaultsOnly, Category = "Pal|NearSkill", meta = (ClampMin = "0.0"))
	float					HitReach = { 80.f };
};
