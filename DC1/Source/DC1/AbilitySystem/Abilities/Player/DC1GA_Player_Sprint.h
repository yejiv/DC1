// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "GameplayEffectTypes.h"
#include "DC1GA_Player_Sprint.generated.h"

class ADC1Player;
class UGameplayEffect;

/**
 * 달리기. 어빌리티가 살아 있는 동안 CMC의 스프린트 의도를 켜 두고 스태미나를 지속 소모한다.
 *
 * 이동 자체는 CMC가 예측으로 소유한다(FSavedMove_DC1의 압축 플래그).
 * 서버에서 스프린트 비트를 켜 봐야 다음 ServerMove가 되돌리므로,
 * LocalPredicted로 돌면서 소유 클라에서 SetSprint를 켜고 소모 GE는 서버에서만 적용한다.
 */
UCLASS()
class DC1_API UDC1GA_Player_Sprint : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	UDC1GA_Player_Sprint(const FObjectInitializer& ObjectInitializer);

private:
	virtual bool	CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void	ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void	EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// 스태미나가 바닥나면 스스로 끝난다
	void			HandleStaminaChanged(const FOnAttributeChangeData& Data);
	// State_OverWeight가 부여되면 끝난다
	void			HandleOverweightTagChanged(const FGameplayTag InTag, int32 InNewCount);

	ADC1Player*	GetPlayer() const;

public:
	// 달리는 동안 유지되는 주기 소모 GE. 비워 두면 소모 없이 달리기만 한다.
	// 회복 지연은 이 GE의 OnCompleteAlways에 차단 GE를 물려서 처리한다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Cost")
	TSubclassOf<UGameplayEffect>	CostEffectClass = { nullptr };

private:
	FActiveGameplayEffectHandle	CostEffectHandle;
	FDelegateHandle			StaminaChangedHandle;
	FDelegateHandle			OverweightTagChangedHandle;
};