// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "DC1GA_Player_PalSphere.generated.h"

class UAnimMontage;
class UDC1EquipComponent;
class ADC1PalSphere;

/**
 * 팰스피어 던지기.
 * 키를 누르고 있는 동안 Loop 섹션으로 조준, 떼면 Throw 섹션으로 점프한다.
 */
UCLASS()
class DC1_API UDC1GA_Player_PalSphere : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	UDC1GA_Player_PalSphere(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageAborted();

private:
	// 던질 스피어가 인벤토리에 1개 이상 있어야 한다
	virtual bool	CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void	ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void	EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UDC1EquipComponent*	GetEquipComponent() const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Anim")
	FGameplayTag	ThrowMontageTag;

public:
	// 손에서 떠나는 지점. 소켓이 없으면 눈높이 앞쪽에서 나간다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|PalSphere")
	FName						ThrowSocketName;

	// 소켓이 없을 때만 쓰는 폴백 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|PalSphere", meta = (ClampMin = "0.0"))
	float						ThrowForwardOffset = { 80.f };

private:
	// 발동할 때 정한 스피어. 조준 중에 등급을 바꿔도 이번 투척은 그대로
	FName						ThrowItemID;
};
