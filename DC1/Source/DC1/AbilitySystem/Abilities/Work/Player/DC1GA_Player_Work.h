// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Work/DC1GA_Base_Work.h"
#include "DC1GA_Player_Work.generated.h"

class UDC1InteractComponent;
class UDC1EquipComponent;

UCLASS()
class DC1_API UDC1GA_Player_Work : public UDC1GA_Base_Work
{
	GENERATED_BODY()

public:
	UDC1GA_Player_Work(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void			EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

public:
	UFUNCTION()
	void					OnInputReleased(float TimeHeld);

	UFUNCTION()
	void					OnMontageCompleted();

	UFUNCTION()
	void					OnMontageAborted();

protected:
	// Work 대상 탐색 기능 구현
	virtual ADC1InteractBuildActor* FindWorkTarget() const override;

	// ADC1InteractBuildActor->AddWorker를 서버 측에서 호출하는 기능 구현
	virtual void			StartWork(ADC1InteractBuildActor * InWorkTarget) override;

	// ADC1InteractBuildActor->RemoveWorker를 서버 측에서 호출하는 기능 구현
	virtual void			StopWork()	override;

	// Work 중 액션 - ex) PlayMontage
	virtual void			OnWorkStarted() override;

private:
	UDC1InteractComponent*	GetInteractComponent() const;
	UDC1EquipComponent*		GetEquipComponent() const;

	void					FinishWork();
	void					HandleBuildCompleted();


protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|EquipTool")
	FGameplayTag	EquipToolTag = { FGameplayTag::EmptyTag };

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Anim")
	FGameplayTag	WorkMontageTag = { FGameplayTag::EmptyTag };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Anim")
	FName			StartSectionName = { "Start" };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Anim")
	FName			EndSectionName = { "End" };

private:
	TWeakObjectPtr<ADC1InteractBuildActor>	WorkTarget = { nullptr };
	FDelegateHandle					BuildCompletedDelegateHandle;
	bool							IsSectionEnd = { false };
	bool							IsPlayingMontage = { false };
};
