// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DC1GameplayEffect_PalSkillCooldown.generated.h"

UCLASS()
class DC1_API UDC1GameplayEffect_PalSkillCooldown : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDC1GameplayEffect_PalSkillCooldown(const FObjectInitializer& ObjectInitializer);
};
