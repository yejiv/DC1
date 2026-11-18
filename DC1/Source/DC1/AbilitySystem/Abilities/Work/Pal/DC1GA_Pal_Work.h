// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Work/DC1GA_Base_Work.h"
#include "DC1GA_Pal_Work.generated.h"

class ADC1Tool;

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
	virtual bool		StartWork(IDC1WorkTargetInterface* InWorkTarget) override;
	// 구독 해제 -> RemoveWorker
	virtual void		StopWork() override;
	// 작업 몽타주
	virtual void		OnWorkStarted() override;

private:
	// 빌드 액터가 작업 배정을 풀었는지 ( 완성 / 아이템 꽉 참 / 해제 / Unassign )
	void				HandlePalWorkersChanged();

	// 서버 - 작업 몽타주 노티파이 타격 → 대상에 1회 ( 진행도형 대상은 무시 )
	UFUNCTION()
	void				HandleWorkHit(FGameplayEventData InPayload);

	UFUNCTION()
	void				OnMontageFinished();

	UFUNCTION()
	void				OnMontageAborted();

	UAnimMontage*		FindWorkMontage() const;

	// 서버 - 작업 종류에 맞는 도구를 손 소켓에 붙인다 ( 원작 BP_ActionArchitecture 와 같은 방식 )
	void				SpawnWorkTool(IDC1WorkTargetInterface* InWorkTarget);

	ADC1PalCharacter*	GetPal() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Anim")
	TMap<FGameplayTag, FGameplayTag>	WorkToMontageTag;

	// 작업 종류( 대상 GetWorkTypeTag ) → 손에 드는 도구. 없으면 맨손
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Tool")
	TMap<FGameplayTag, TSubclassOf<ADC1Tool>>	WorkTools;

	// 도구를 붙일 팰 메시 소켓
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Tool")
	FName								ToolSocketName = { TEXT("Socket_Weapon_R") };

private:
	FDelegateHandle						PalWorkersChangedHandle;

	// 이번 작업에서 든 도구 ( StopWork 에서 지움 )
	TWeakObjectPtr<ADC1Tool>			WorkTool;
};
