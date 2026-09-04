// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DC1GameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class DC1_API UDC1GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UDC1GameplayAbility(const FObjectInitializer& ObjectInitializer);

protected:
	virtual bool		CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void		ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void		EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// 코스트 GE의 SetByCaller.StaminaCost 칸을 StaminaCost로 채워서 적용한다.
	// Super의 CheckCost는 GE 클래스만 보고 계산해 SetByCaller를 0으로 취급하므로 잔량 검사도 여기서 한다
	virtual bool		CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void		ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

public:
	// 이 어빌리티가 쓰는 스태미나. 0이면 소모하지 않는다.
	// 양수로 적는다(적용 시점에 부호를 뒤집는다). Cost Gameplay Effect Class 를 같이 물려야 동작한다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Cost", meta = (ClampMin = "0.0"))
	float				StaminaCost = { 0.f };
};
