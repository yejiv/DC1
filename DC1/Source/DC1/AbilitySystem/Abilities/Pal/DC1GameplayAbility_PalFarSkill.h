// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalSkill.h"
#include "DC1GameplayAbility_PalFarSkill.generated.h"

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
	// PalData.Projectiles 조회 키
	UPROPERTY(EditDefaultsOnly, Category = "Pal|FarSkill", meta = (Categories = "Ability.Pal"))
	FGameplayTag			ProjectileTag;

	// 발사 위치
	UPROPERTY(EditDefaultsOnly, Category = "Pal|FarSkill")
	FName					MuzzleSocketName = { TEXT("Muzzle") };

	// 소켓이 없을 때만 쓰는 폴백 오프셋
	UPROPERTY(EditDefaultsOnly, Category = "Pal|FarSkill", meta = (ClampMin = "0.0"))
	float					MuzzleForwardOffset = { 60.f };

	UPROPERTY(EditDefaultsOnly, Category = "Pal|FarSkill")
	float					MuzzleHeightOffset = { 60.f };
};
