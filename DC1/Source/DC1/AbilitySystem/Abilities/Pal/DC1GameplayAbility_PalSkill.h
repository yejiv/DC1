// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "Define/DC1Enums.h"
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

	// 아래 값은 전부 SkillChart 의 줄이 있으면 그것을, 없으면 이 BP 의 값을 돌려준다.
	// 표가 비어 있어도 예전과 똑같이 동작하게 하는 폴백이다
	float					GetMaxRange() const { return bHasSkillDef ? SkillDef.MaxRange : MaxRange; }
	float					GetApproachRange() const { return bHasSkillDef ? SkillDef.ApproachRange : ApproachRange; }
	float					GetMinRange() const { return bHasSkillDef ? SkillDef.MinRange : MinRange; }
	float					GetSkillPower() const { return bHasSkillDef ? SkillDef.Power : SkillPower; }
	EDC1PalElement			GetSkillElement() const { return bHasSkillDef ? SkillDef.Element : SkillElement; }

protected:
	virtual void			ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void			PerformSkillEffect() PURE_VIRTUAL(UDC1GameplayAbility_PalSkill::PerformSkillEffect, );

	UFUNCTION()
	void					OnMontageFinished();

	UFUNCTION()
	void					OnMontageAborted();

	UFUNCTION()
	void					OnTriggerEventReceived(FGameplayEventData Payload);

	// 컨트롤러가 고른 스킬 줄을 가져온다. ActivateAbility 초입에서 한 번만 부른다.
	// 표에 없거나 컨트롤러가 없으면 bHasSkillDef 가 false 로 남고 이 BP 값을 쓴다
	void					CacheSkillDef();

protected:
	// 이번 발동에 쓸 스킬 줄. 발동할 때마다 다시 채운다.
	// ProjectileClass 를 들고 있어서 UPROPERTY 로 잡아둔다
	UPROPERTY()
	FDC1PalSkillDef			SkillDef;
	bool					bHasSkillDef = { false };

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Skill", meta = (Categories = "Anim.Pal"))
	FGameplayTag			MontageTag;

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Skill", meta = (Categories = "Event"))
	FGameplayTag			TriggerEventTag;

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Skill", meta = (ClampMin = "0.0"))
	float					MaxRange = { 150.f };		// 공격이 닿는 한계

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Skill", meta = (ClampMin = "0.0"))
	float					ApproachRange = { 150.f };	// 접근해서 멈출 자리

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Skill", meta = (ClampMin = "0.0"))
	float					MinRange = { 0.f };			// 물러날 거리

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Skill", meta = (ClampMin = "0.0"))
	float					SkillPower = { 12.f };		// 스킬의 세기

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Skill")
	EDC1PalElement			SkillElement = { EDC1PalElement::Neutral };
};
