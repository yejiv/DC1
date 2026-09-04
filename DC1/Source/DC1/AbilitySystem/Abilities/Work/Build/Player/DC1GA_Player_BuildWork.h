// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Work/Build/DC1GA_Base_BuildWork.h"
#include "DC1GA_Player_BuildWork.generated.h"

class UDC1InteractComponent;

UCLASS()
class DC1_API UDC1GA_Player_BuildWork : public UDC1GA_Base_BuildWork
{
	GENERATED_BODY()
	
public:
	UDC1GA_Player_BuildWork(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION()
	void					OnInputReleased(float TimeHeld);

	UFUNCTION()
	void					OnMontageCompleted();

	UFUNCTION()
	void					OnMontageAborted();

protected:
	// BuildActor 탐색 기능 구현
	virtual ADC1InteractBuildActor* FindBuildTarget() const override;

	// ADC1InteractBuildActor->AddWorker를 서버 측에서 호출하는 기능 구현
	virtual void			StartWork(ADC1InteractBuildActor * InBuildTarget) override;

	// ADC1InteractBuildActor->RemoveWorker를 서버 측에서 호출하는 기능 구현
	virtual void			StopWork()	override;

	// Work 중 액션 - ex) PlayMontage
	virtual void			OnWorkStarted() override;

private:
	UDC1InteractComponent*	GetInteractComponent() const;
	
	void					FinishWork();
	void					HandleBuildCompleted();


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
