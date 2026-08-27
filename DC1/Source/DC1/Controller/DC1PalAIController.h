// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "DC1PalAIController.generated.h"

class UStateTreeAIComponent;
class UStateTree;
class UAISenseConfig_Sight;

/**
 *
 */
UCLASS()
class DC1_API ADC1PalAIController : public AAIController
{
	GENERATED_BODY()

public:
	ADC1PalAIController();

	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	virtual FGenericTeamId GetGenericTeamId() const override;

	void					UpdateTarget();

	AActor*					GetCurrentTarget() const { return ::IsValid(CurrentTarget) ? CurrentTarget.Get() : nullptr; }

	void					StopBehavior();
	void					TryStartBehavior();

	// 진영이 바뀌면 지각과 행동 트리 다시.
	void					HandleTeamChanged();

	AActor*					GetFollowTarget() const;

	// 싸우는 타겟
	AActor*					GetFollowTargetEnemy() const;

	void					UpdateSelectedSkill();
	void					ClearSelectedSkill();

	const FGameplayTag&		GetSelectedSkillTag() const { return SelectedSkillTag; }
	float					GetSelectedMaxRange() const { return SelectedMaxRange; }
	float					GetSelectedApproachRange() const { return SelectedApproachRange; }

	void					ProvokeBy(AActor* InInstigator);
	bool					IsProvoked() const { return ::IsValid(ProvokedTarget); }

	// 주인 기준 어느 쪽에 설지. 추종과 주시 상태가 공유하므로 태스크가 아니라 여기 둔다
	const FVector&			GetFollowAnchor() const { return FollowAnchor; }

	// 화면 밖에 오래 있었으면 보이는 자리로 설 방향을 다시 고른다. 바뀌면 true
	bool					UpdateFollowAnchor(float DeltaTime);

	// 배회 기준점. 빙의 시점의 위치
	const FVector&			GetHomeLocation() const { return HomeLocation; }
	void					SetHomeLocation(const FVector& InLocation) { HomeLocation = InLocation; }

protected:
	virtual void			BeginPlay() override;
	virtual void			OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void					OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	
	void					SetCurrentTarget(AActor* NewTarget);

	void					ApplyPerceptionSettings();

	void					RefreshPerception();

	// 진영에 맞는 행동 트리 적용
	void					ApplyStateTreeForTeam();

	void					ClearTargetState();

protected:
	// 진영별 행동 트리. 컨트롤러 BP에서 지정한다
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UStateTree>	WildStateTree = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UStateTree>	PartnerStateTree = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent = { nullptr };

	// 화면 밖에 머무는 시간
	UPROPERTY(EditDefaultsOnly, Category = "AI|Follow", meta = (ClampMin = "0.0"))
	float					OutOfViewDelay = { 3.f };

	// 화면 가장자리 여유 비율. 0.12면 안쪽 76퍼센트가 돼야 허용
	UPROPERTY(EditDefaultsOnly, Category = "AI|Follow", meta = (ClampMin = "0.0", ClampMax = "0.45"))
	float					ScreenMargin = { 0.12f };

	FVector					FollowAnchor = { FVector::ZeroVector };
	float					OutOfViewTime = { 0.f };

	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig = { nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<AActor>				CurrentTarget = { nullptr };

	// 현재 감지 중인 적들. 죽으면 UpdateTarget에서 걸러짐.
	TArray<TWeakObjectPtr<AActor>>	SensedTargets;

	UPROPERTY()
	TObjectPtr<AActor>				ProvokedTarget = { nullptr };

	UPROPERTY()
	TObjectPtr<AActor>				LastProvokedBy = { nullptr };

	FGameplayTag					SelectedSkillTag;
	float							SelectedMaxRange = { 0.f };
	float							SelectedApproachRange = { 0.f };

	FVector							HomeLocation = { FVector::ZeroVector };
	float							LastSeenTime = { 0.f };

	// 놓친 뒤 추격 유지하는 시간
	float							TargetMemoryDuration = { 3.f };

	// 시야에서 놓친 뒤 따라가는 시간
	float							ProvokeMemoryDuration = { 10.f };
};
