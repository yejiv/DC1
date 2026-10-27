// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "DC1PalStateTreeTasks.generated.h"

class AActor;
class ADC1PalAIController;

USTRUCT()
struct FDC1PalActivateAbilityTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AActor>	Actor = { nullptr };

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (Categories = "Ability"))
	FGameplayTag		AbilityTag;

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (Categories = "State.Pal"))
	FGameplayTag		ActiveStateTag;
};


USTRUCT(DisplayName = "Pal Activate Ability", Category = "Pal")
struct FDC1PalActivateAbilityTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FDC1PalActivateAbilityTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};


USTRUCT()
struct FDC1PalActivateSkillTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ADC1PalAIController>	AIController = { nullptr };
	
	UPROPERTY()
	FGameplayTag		ActivatedSkillTag;// EnterState에서 실제로 발동한 태그
};


//컨트롤러가 고른 스킬을 발동
USTRUCT(DisplayName = "Pal Activate Selected Skill", Category = "Pal")
struct FDC1PalActivateSkillTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FDC1PalActivateSkillTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};


USTRUCT()
struct FDC1PalWanderTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ADC1PalAIController>	AIController = { nullptr };

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0"))
	float							AcceptanceRadius = { 60.f };	//도착으로 치는 거리
};


USTRUCT(DisplayName = "Pal Wander", Category = "Pal")
struct FDC1PalWanderTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FDC1PalWanderTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};


USTRUCT()
struct FDC1PalFollowTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ADC1PalAIController>	AIController = { nullptr };

	
	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "1.0"))
	float							CatchUpDistance = { 400.f };	// 따라잡기 시작하는 거리

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.1"))
	float							SpeedScale = { 1.3f };			// 따라잡을 때 주인 속도에 곱할 값

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0"))
	float							TeleportDistance = { 2500.f };	// 순간이동. 데드존(1000)보다 커야 데드존이 먼저 걸린다

	UPROPERTY()
	FVector							LastRequestedLocation = { FVector::ZeroVector };

	UPROPERTY()
	bool							bCatchingUp = { false };
};


USTRUCT(DisplayName = "Pal Follow", Category = "Pal")
struct FDC1PalFollowTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FDC1PalFollowTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};


USTRUCT()
struct FDC1PalFaceOwnerTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ADC1PalAIController>	AIController = { nullptr };

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "1.0"))
	float							TurnRate = { 150.f };

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0"))
	float							FollowDeadzone = { 1000.f };	//이상 움직이면 따라감
};



USTRUCT(DisplayName = "Pal Face Owner", Category = "Pal")
struct FDC1PalFaceOwnerTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FDC1PalFaceOwnerTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};



USTRUCT()
struct FDC1PalRetreatTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ADC1PalAIController>	AIController = { nullptr };

	// 한 번에 물러나는 거리
	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0"))
	float							StepDistance = { 600.f };

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float							LateralAngleMax = { 30.f };	//직선 말고 +-30

	// 도착으로 치는 거리
	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0"))
	float							AcceptanceRadius = { 60.f };

	UPROPERTY(EditAnywhere, Category = Parameter)
	bool							bRepeat = { false };	//계속 도주하는지 플래그

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0"))
	float							SafeDistance = { 1500.f };	//도주 끝

	// 진입 시점의 위협. 표적이 기억에서 지워져도 이걸로 계속 달아난다
	UPROPERTY()
	TWeakObjectPtr<AActor>			Threat;
};


// 반대쪽으로 물러나기
USTRUCT(DisplayName = "Pal Retreat", Category = "Pal")
struct FDC1PalRetreatTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FDC1PalRetreatTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};


USTRUCT()
struct FDC1PalFaceTargetTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ADC1PalAIController>	AIController = { nullptr };

	// 표적이 정면에서 이 각도 안이면 통과
	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float							AcceptAngle = { 10.f };

	// 이 시간 안에 못 돌면 그 방향 그대로 통과
	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0"))
	float							GiveUpTime = { 1.f };

	UPROPERTY()
	float							ElapsedTime = { 0.f };
};


// 공격 전에 표적 쪽으로 돈다 (BTTask_RotateToFaceBBEntry 와 같은 역할). Failed 는 내지 않는다
USTRUCT(DisplayName = "Pal Face Target", Category = "Pal")
struct FDC1PalFaceTargetTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FDC1PalFaceTargetTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};


USTRUCT()
struct FDC1PalWorkTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ADC1PalAIController>	AIController = { nullptr };

	// 작업 대상 표면에서 이만큼 안이면 도착
	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0"))
	float							AcceptanceRadius = { 50.f };

	UPROPERTY()
	bool							bArrived = { false };
};


// 거점 작업
USTRUCT(DisplayName = "Pal Work", Category = "Pal")
struct FDC1PalWorkTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FDC1PalWorkTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
