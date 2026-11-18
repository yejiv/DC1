// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "DC1PalAIController.generated.h"

class UStateTreeAIComponent;
class UStateTree;
class UAISenseConfig_Sight;

DECLARE_DELEGATE_OneParam(FDC1OnRideApproachFinished, bool /*bArrived*/);

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
	virtual void Tick(float DeltaSeconds) override;

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

	// 고른 스킬. SkillChart 의 키이자 발동 태그다 (스킬마다 스펙을 따로 받는다)
	const FGameplayTag&		GetSelectedSkillTag() const { return SelectedSkillTag; }

	float					GetSelectedMaxRange() const { return SelectedMaxRange; }
	float					GetSelectedApproachRange() const { return SelectedApproachRange; }
	float					GetSelectedMinRange() const { return SelectedMinRange; }

	// 물러난 직후에는 false. 붙어오는 상대에게서 계속 물러나는 걸 막는다
	bool					CanBackoff() const;

	// Backoff 태스크가 끝날 때 알린다. 쿨다운이 여기서 시작된다
	void					NotifyBackoffPerformed();

	void					ProvokeBy(AActor* InInstigator);
	bool					IsProvoked() const { return ::IsValid(ProvokedTarget); }

	// 주인이 때린 상대. 그 상대가 주인과 싸우는 동안 같이 싸운다
	void					NotifyOwnerHit(AActor* InVictim);

	// 주인 기준 월드 오프셋
	const FVector&			GetFollowAnchor() const { return FollowAnchor; }
	bool					UpdateFollowAnchor(float DeltaTime);
	// 다음 UpdateFollowAnchor 에서 지금 카메라 기준으로 다시 설정
	void					ResetFollowAnchor();

	// 배회 기준점. 빙의 시점의 위치, 거점 팰은 팰박스 위치
	const FVector&			GetHomeLocation() const { return HomeLocation; }

	// 배회 반경. 거점 팰은 거점 범위
	float					GetWanderRadius() const;

	// 실패 시 트리 복구
	bool					RequestRideApproach(APawn* InRider, float InAcceptanceRadius, FDC1OnRideApproachFinished InOnFinished);
	void					CancelRideApproach();
	bool					IsRideApproaching() const { return OnRideApproachFinished.IsBound(); }

protected:
	virtual void			BeginPlay() override;
	virtual void			OnPossess(APawn* InPawn) override;
	virtual void			OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	UFUNCTION()
	void					OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	
	void					SetCurrentTarget(AActor* NewTarget);

	void					ApplyPerceptionSettings();

	void					RefreshPerception();

	// 진영에 맞는 행동 트리 적용
	void					ApplyStateTreeForTeam();

	void					ClearTargetState();
	void					BeginReturnHome();
	void					FinishReturnHome(bool bArrived);

protected:
	// 진영별 행동 트리. 컨트롤러 BP에서 지정한다
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UStateTree>	WildStateTree = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UStateTree>	PartnerStateTree = { nullptr };

	// 거점 팰. 진영은 Player 지만 주인을 따라가지 않는다
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UStateTree>	BaseStateTree = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent = { nullptr };

	// 화면 밖에 머무는 시간
	UPROPERTY(EditDefaultsOnly, Category = "AI|Follow", meta = (ClampMin = "0.0"))
	float					OutOfViewDelay = { 3.f };

	// 화면 가장자리 여유 비율.
	UPROPERTY(EditDefaultsOnly, Category = "AI|Follow", meta = (ClampMin = "0.0", ClampMax = "0.45"))
	float					ScreenMargin = { 0.12f }; // 0.12면 안쪽 76퍼센트가 돼야 허용

	// 카메라 기준 로컬 오프셋
	UPROPERTY(EditDefaultsOnly, Category = "AI|Follow")
	FVector2D				FollowOffset = { 1200.f, 570.f };

	// 매 틱 다시 구하면 카메라를 돌리는 즉시 팰이 따라 돈다. 지연을 만들려면 붙들고 있어야 한다
	FVector					FollowAnchor = { FVector::ZeroVector };
	float					OutOfViewTime = { 0.f };

	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig = { nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<AActor>				CurrentTarget = { nullptr };

	// 현재 감지 중인 적들
	TArray<TWeakObjectPtr<AActor>>	SensedTargets;

	// 지금 싸울 상대
	UPROPERTY()
	TObjectPtr<AActor>				ProvokedTarget = { nullptr };

	// 나를 때린 적 있는 상대
	TArray<TWeakObjectPtr<AActor>>	Aggressors;

	// 주인이 때린 상대. 최근에 때린 상대가 뒤
	TArray<TWeakObjectPtr<AActor>>	OwnerHitTargets;

	FGameplayTag					SelectedSkillTag;

	float							SelectedMaxRange = { 0.f };
	float							SelectedApproachRange = { 0.f };
	float							SelectedMinRange = { 0.f };

	// 한 번 물러난 뒤 다시 물러나기까지의 최소 간격
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Combat", meta = (ClampMin = "0.0"))
	float							BackoffCooldown = { 3.f };

	float							LastBackoffTime = { -1000.f };

	// 야생 팰이 전투 중 배회 중심에서 이 거리 이상 벗어나면 전투를 끊고 복귀한다.
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Combat", meta = (ClampMin = "0.0", Units = "cm"))
	float							CombatLeashDistance = { 5000.f };

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Combat", meta = (ClampMin = "0.0", Units = "cm"))
	float							ReturnHomeAcceptanceRadius = { 200.f };

	FVector							HomeLocation = { FVector::ZeroVector };
	bool							bReturningHome = { false };
	FAIRequestID					ReturnHomeRequestID;
	float							LastSeenTime = { 0.f };

	// 놓친 뒤 추격 유지하는 시간
	float							TargetMemoryDuration = { 3.f };

	// 시야에서 놓친 뒤 따라가는 시간
	float							ProvokeMemoryDuration = { 10.f };

	// 바인딩 = 달려가는 중
	FDC1OnRideApproachFinished		OnRideApproachFinished;
	FAIRequestID					RideApproachRequestID;
};
