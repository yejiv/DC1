// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Work/DC1GA_Base_Work.h"
#include "DC1GA_Pal_Work.generated.h"

/**
 * 
 */
UCLASS()
class DC1_API UDC1GA_Pal_Work : public UDC1GA_Base_Work
{
	GENERATED_BODY()
	
public:
	UDC1GA_Pal_Work(const FObjectInitializer& ObjectInitializer);

protected:
	// 작업 배정 확인 -> 해제 감지 구독 -> Add Worker
	virtual bool		StartWork(ADC1InteractBuildActor* InWorkTarget) override;
	// 구독 해제 -> RemoveWorker
	virtual void		StopWork() override;
	// 작업 몽타주
	virtual void		OnWorkStarted() override;

private:
	// 빌드 액터가 작업 배정을 풀었는지 ( 완성 / 아이템 꽉 참 / 해제 / Unassign )
	void				HandlePalWorkersChanged();

	UFUNCTION()
	void				OnMontageFinished();

	UFUNCTION()
	void				OnMontageAborted();

	UAnimMontage*		FindWorkMontage() const;

	ADC1PalCharacter*	GetPal() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Anim")
	TMap<FGameplayTag, FGameplayTag>	WorkToMontageTag;

private:
	FDelegateHandle						PalWorkersChangedHandle;
};
