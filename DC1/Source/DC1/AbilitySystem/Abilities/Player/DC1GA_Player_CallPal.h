// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "DC1GA_Player_CallPal.generated.h"

class UAnimMontage;
class USoundBase;
class ADC1PalSphere;

/**
 * 팰 소환/회수 ( E ).
 * 소환 중인 팰이 있으면 회수, 없으면 소환한다.
 */
UCLASS()
class DC1_API UDC1GA_Player_CallPal : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	UDC1GA_Player_CallPal(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION()
	void OnMontageCompleted();
	UFUNCTION()
	void OnMontageAborted();

private:
	virtual bool	CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void	ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void	EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

public:
	// 소환
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Anim")
	TObjectPtr<UAnimMontage>	DeployMontage = { nullptr };

	// 회수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Anim")
	TObjectPtr<UAnimMontage>	RetrieveMontage = { nullptr };

	// 소환 / 회수 사운드
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Sound")
	TObjectPtr<USoundBase>		DeploySound = { nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Sound")
	TObjectPtr<USoundBase>		RetrieveSound = { nullptr };

	// 소환 / 회수 몽타주 동안 손에 들고 있을 스피어 ( 표시용, 던지지 않는다 )
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|PalSphere")
	TSubclassOf<ADC1PalSphere>	HeldSphereClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|PalSphere")
	FName						HeldSocketName;

	// 손 소켓 기준 위치 보정 ( 팰스피어 던지기와 같은 값을 쓰면 된다 )
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|PalSphere")
	FVector						HeldSphereOffset = { FVector::ZeroVector };

private:
	// 서버: 몽타주 동안 들고 있는 스피어. EndAbility 에서 지운다
	TWeakObjectPtr<ADC1PalSphere>	HeldSphere;
};
