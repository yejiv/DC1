// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalSkill.h"
#include "Templates/SubclassOf.h"
#include "DC1GameplayAbility_PalFarSkill.generated.h"

class ADC1PalProjectile;

/**
 *
 *
 */
UCLASS()
class DC1_API UDC1GameplayAbility_PalFarSkill : public UDC1GameplayAbility_PalSkill
{
	GENERATED_BODY()

public:
	UDC1GameplayAbility_PalFarSkill(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void			PerformSkillEffect() override;

protected:
	// 속도, 중력, 메시는 BP에서 설정
	UPROPERTY(EditDefaultsOnly, Category = "Pal|FarSkill")
	TSubclassOf<ADC1PalProjectile>	ProjectileClass;

	// 발사 위치 오프셋.
	UPROPERTY(EditDefaultsOnly, Category = "Pal|FarSkill", meta = (ClampMin = "0.0"))
	float					MuzzleForwardOffset = { 60.f };

	UPROPERTY(EditDefaultsOnly, Category = "Pal|FarSkill")
	float					MuzzleHeightOffset = { 60.f };
};
