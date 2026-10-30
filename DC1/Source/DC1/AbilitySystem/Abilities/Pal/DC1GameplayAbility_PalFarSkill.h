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
	virtual void			EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	void					FireProjectile();

	// 연사 타이머가 ShotInterval 마다 호출
	void					FireNextShot();

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

	UPROPERTY(EditDefaultsOnly, Category = "Pal|FarSkill", meta = (ClampMin = "0.0"))
	float					AboveHeadOffset = { 50.f };

private:
	UPROPERTY()
	TSubclassOf<ADC1PalProjectile>	ActiveProjectileClass;

	int32					RemainingShots = { 0 };
	FTimerHandle			ShotTimerHandle;
};
