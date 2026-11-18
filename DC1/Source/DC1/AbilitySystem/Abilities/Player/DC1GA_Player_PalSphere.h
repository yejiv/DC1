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

	// 서버: 던질 스피어를 만들어 ThrowSocketName 소켓(손)에 붙인다
	ADC1PalSphere*		SpawnHeldSphere();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Anim")
	FGameplayTag	ThrowMontageTag;

public:
	// 조준하는 동안 스피어를 들고 있다가 던지는 손 소켓
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|PalSphere")
	FName						ThrowSocketName;

	// 손 소켓 기준 위치 보정 (소켓 축 기준). 손바닥 쪽으로 옮길 때 쓴다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|PalSphere")
	FVector						HeldSphereOffset = { FVector::ZeroVector };

private:
	// 발동할 때 정한 스피어. 조준 중에 등급을 바꿔도 이번 투척은 그대로
	FName						ThrowItemID;

	// 조준하는 동안 손에 들고 있는 스피어 (서버). 던지면 비우고, 던지기 전에 끝나면 지운다
	TWeakObjectPtr<ADC1PalSphere>	HeldSphere;
};
