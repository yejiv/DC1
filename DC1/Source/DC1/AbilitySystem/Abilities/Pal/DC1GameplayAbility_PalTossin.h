// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalSkill.h"
#include "DC1GameplayAbility_PalTossin.generated.h"

/**
 *
 */
UCLASS()
class DC1_API UDC1GameplayAbility_PalTossin : public UDC1GameplayAbility_PalSkill
{
	GENERATED_BODY()

public:
	UDC1GameplayAbility_PalTossin(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void			PerformSkillEffect() override;
	virtual void			EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	// 돌진 중 전방을 훑어 적대 대상에 데미지. 타이머가 SweepInterval마다 부른다
	void					SweepAndApplyDamage();

	UFUNCTION()
	void					StopCharge();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Tossin", meta = (ClampMin = "0.0"))
	float					ChargeSpeed = { 900.f };

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Tossin", meta = (ClampMin = "0.0"))
	float					ChargeDuration = { 1.2f }; // 이 시간 x 속도 = 돌진 거리 (몽타주 Loop 구간 길이)

	// 돌진 중 전방 판정 길이
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Tossin", meta = (ClampMin = "0.0"))
	float					ChargeTraceLength = { 100.f };

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Tossin", meta = (ClampMin = "0.01"))
	float					SweepInterval = { 0.05f };

	// 전방 판정 구체의 반지름
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Tossin", meta = (ClampMin = "0.0"))
	float					TraceRadius = { 70.f };

private:
	UPROPERTY()
	TSet<TObjectPtr<AActor>>	DamagedActors; // 돌진 한 번에 대상당 한 번만 때린다

	FTimerHandle				SweepTimerHandle;
};
