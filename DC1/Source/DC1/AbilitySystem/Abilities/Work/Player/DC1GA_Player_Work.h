// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Work/DC1GA_Base_Work.h"
#include "DC1GA_Player_Work.generated.h"

class UDC1InteractComponent;
class UDC1EquipComponent;
struct FGameplayAbilityTargetDataHandle;

// 작업 종류( 대상 GetWorkTypeTag )별 도구 / 몽타주
USTRUCT(BlueprintType)
struct FDC1PlayerWorkSet
{
	GENERATED_BODY()

	// 비어 있으면 무기를 그대로 둔다 ( 맨손 작업 )
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Item.Tool"))
	FGameplayTag	ToolTag = { FGameplayTag::EmptyTag };

	// DA_Anim_Player 몽타주 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Anim.Action.Working"))
	FGameplayTag	MontageTag = { FGameplayTag::EmptyTag };

	// 시작 섹션 - 몽타주에 없으면 처음부터 재생
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName			StartSectionName = { "Start" };

	// 키를 뗐을 때 넘어갈 섹션 - 몽타주에 없으면 현재 루프를 마무리하고 종료
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName			EndSectionName = { "End" };
};

/*
 * 플레이어 작업 - F 홀드 하나로 모든 작업. 도구 / 몽타주는 대상의 작업 종류로 결정
 *  클라 : 대상을 TargetData로 서버 GA에 전송 → 예측 실행
 *  서버 : TargetData 수신 → 거리 검증 → AddWorker
 */
UCLASS()
class DC1_API UDC1GA_Player_Work : public UDC1GA_Base_Work
{
	GENERATED_BODY()

public:
	UDC1GA_Player_Work(const FObjectInitializer& ObjectInitializer);

	// 로컬 - 대상 작업 종류가 WorkSets에 없으면 false ( 플레이어가 할 수 없는 작업 → 프롬프트에서도 숨김 )
	virtual bool			CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

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
	// 클라 : 서버에 대상 전송 + 대상 잠금 / 완성 구독 → BeginWork
	virtual void			OnLocalWorkTargetFound(IDC1WorkTargetInterface* InWorkTarget) override;

	// 서버( 원격 클라 ) : TargetData 수신 대기
	virtual void			WaitWorkTarget() override;

	// 서버 : 거리 검증 → AddWorker
	virtual bool			StartWork(IDC1WorkTargetInterface* InWorkTarget) override;

	// 서버 : RemoveWorker
	virtual void			StopWork()	override;

	// 양쪽 : 작업 종류 → 도구 / Release 대기 / 몽타주
	virtual void			OnWorkStarted() override;

private:
	void					OnWorkTargetDataReceived(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag InActivationTag);

	UDC1InteractComponent*	GetInteractComponent() const;
	UDC1EquipComponent*		GetEquipComponent() const;

	void					FinishWork();
	void					HandleBuildCompleted();

protected:
	// 작업 종류( Work.X ) → 도구 / 몽타주. 매핑이 없는 작업은 플레이어가 할 수 없음
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Work", meta = (Categories = "Work"))
	TMap<FGameplayTag, FDC1PlayerWorkSet>	WorkSets;

private:
	// 이번 작업에서 장착한 도구 ( EndAbility 해제용 )
	FGameplayTag					CurrentToolTag = { FGameplayTag::EmptyTag };

	// 이번 작업 몽타주의 종료 섹션 ( FinishWork 용 )
	FName							CurrentEndSectionName = { NAME_None };

	FDelegateHandle					BuildCompletedDelegateHandle;
	FDelegateHandle					TargetDataDelegateHandle;
	bool							IsSectionEnd = { false };
	bool							IsPlayingMontage = { false };
};
