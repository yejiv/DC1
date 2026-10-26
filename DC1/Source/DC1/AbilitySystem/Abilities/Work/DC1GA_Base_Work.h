// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/DC1GameplayAbility.h"

#include "Interface/DC1WorkTargetInterface.h"
#include "UObject/WeakInterfacePtr.h"

#include "DC1GA_Base_Work.generated.h"

/*
 * 작업 GA 공통 흐름
 *  로컬( 대상을 아는 쪽 ) : FindWorkTarget → OnLocalWorkTargetFound → BeginWork
 *  로컬이 아닌 서버       : WaitWorkTarget ( 클라가 보낸 대상 수신 ) → BeginWork
 *  BeginWork              : WorkTarget 확정 → 서버면 StartWork → OnWorkStarted ( 양쪽 )
 *  EndAbility             : 서버면 StopWork
 */
UCLASS(Abstract)
class DC1_API UDC1GA_Base_Work : public UDC1GameplayAbility
{
	GENERATED_BODY()


public:
	UDC1GA_Base_Work(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void			ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void			EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// 대상 확정 - 서버면 StartWork( 실패 시 End ), 이어서 OnWorkStarted
	void					BeginWork(IDC1WorkTargetInterface* InWorkTarget);

	// 로컬에서 대상을 찾은 직후 - 기본은 바로 BeginWork ( 펠 = 서버 로컬 )
	virtual void			OnLocalWorkTargetFound(IDC1WorkTargetInterface* InWorkTarget) { BeginWork(InWorkTarget); }

	// 로컬이 아닌 인스턴스( 원격 클라의 서버 GA ) - 대상 수신 대기. 기본은 대상을 알 방법이 없으므로 End
	virtual void			WaitWorkTarget();

private:
	// Work 대상 - 아바타( ADC1Character )의 GetCurrentWorkTarget
	IDC1WorkTargetInterface*	FindWorkTarget() const;

// PURE_VIRTUAL
protected:
	// 서버 - 검증 후 대상->AddWorker. false면 End
	virtual bool			StartWork(IDC1WorkTargetInterface* InWorkTarget) PURE_VIRTUAL(UDC1GA_Base_Work::StartWork, return false;);

	// 서버 - 대상->RemoveWorker
	virtual void			StopWork()	PURE_VIRTUAL(UDC1GA_Base_Work::StopWork, );

	// 대상 확정 후 액션 ( 양쪽 ) - ex) 도구 / PlayMontage
	virtual void			OnWorkStarted() {};

protected:
	// BeginWork에서 확정, EndAbility에서 비움
	TWeakInterfacePtr<IDC1WorkTargetInterface>	WorkTarget = { nullptr };
};
