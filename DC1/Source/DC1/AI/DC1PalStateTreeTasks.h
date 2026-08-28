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

	
	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0"))
	float							AnchorDistance = { 700.f };		// 주인과 오프셋

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "1.0"))
	float							CatchUpDistance = { 400.f };	// 따라잡기 시작하는 거리

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.1"))
	float							SpeedScale = { 1.3f };			// 따라잡을 때 주인 속도에 곱할 값

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0"))
	float							TeleportDistance = { 1500.f };	// 순간이동

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

	// 회전 속도
	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "1.0"))
	float							TurnRate = { 150.f };
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
struct FDC1PalFleeTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ADC1PalAIController>	AIController = { nullptr };

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0"))
	float							AcceptanceRadius = { 100.f };	//도착으로 치는 거리
};


USTRUCT(DisplayName = "Pal Flee", Category = "Pal")
struct FDC1PalFleeTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FDC1PalFleeTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};


USTRUCT()
struct FDC1PalBackoffTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ADC1PalAIController>	AIController = { nullptr };

	// 한 번에 물러나는 거리.
	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0"))
	float							StepDistance = { 300.f };

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (ClampMin = "0.0"))
	float							AcceptanceRadius = { 60.f };	//도착으로 치는 거리
};


USTRUCT(DisplayName = "Pal Backoff", Category = "Pal")
struct FDC1PalBackoffTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FDC1PalBackoffTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
