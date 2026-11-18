// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "DC1GA_Player_Dodge.generated.h"

class ADC1Player;

/**
 * 회피. 땅에 붙어 있을 때만 발동한다.
 * 조준 중이면 4방향 스텝, 그 외에는 앞구르기. 웅크림은 풀고 둘 중 하나로 간다.
 * 어빌리티가 살아 있는 동안 State.Dodging / State.Invincible이 붙는다.
 */
UCLASS()
class DC1_API UDC1GA_Player_Dodge : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	UDC1GA_Player_Dodge(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageAborted();

private:
	virtual bool	CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void	ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void	EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// 조준 중이면 Step, 아니면 Roll
	FGameplayTag	SelectMontageTag() const;

	// 로컬 입력 방향으로 스텝 섹션을 고른다. 입력이 없으면 Bwd
	FName			SelectStepSection() const;

	void				Compute_InputDirection();

	// 기본 상태 전용. 루트모션이 시작되기 전에 입력 방향으로 즉시 돌려준다.
	// 조준 중에는 카메라 방향이 정면이므로 호출하면 안 된다
	void				FaceInputDirection();

	ADC1Player*		GetPlayer() const;
	bool				CheckAiming() const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Anim")
	FGameplayTag	RollMontageTag;

	// 4방향이 Forward / Backward / Left / Right 섹션으로 들어있는 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Anim")
	FGameplayTag	StepMontageTag;


	UPROPERTY(EditDefaultsOnly, Category = "Ability|Anim|Move")
	float				Strength = {1000.f};
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Anim|Move")
	float				Duration = {0.5f};

private:
	FVector			InputLocalDirection;
	FVector			InputWorldDirection;

	bool				IsAiming = {};

	float				RollStrength = { 1000.f };
	float				StepStrength = { 500.f };
};