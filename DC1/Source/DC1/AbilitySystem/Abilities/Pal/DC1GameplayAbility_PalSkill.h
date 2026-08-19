// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "DC1GameplayAbility_PalSkill.generated.h"

/**
 * 
 * 
 */
UCLASS(Abstract)
class DC1_API UDC1GameplayAbility_PalSkill : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	float				GetMinRange() const { return MinRange; }
	float				GetMaxRange() const { return MaxRange; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Skill", meta = (ClampMin = "0.0"))
	float				MinRange = { 0.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pal|Skill", meta = (ClampMin = "0.0"))
	float				MaxRange = { 150.f };
};
