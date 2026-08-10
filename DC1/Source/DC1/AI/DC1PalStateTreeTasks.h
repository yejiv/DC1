// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "DC1PalStateTreeTasks.generated.h"

class AActor;

USTRUCT()
struct FDC1PalActivateAbilityTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AActor>	Actor = { nullptr };

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (Categories = "Ability"))
	FGameplayTag		AbilityTag;

	UPROPERTY(EditAnywhere, Category = Parameter, meta = (Categories = "Pal.State"))
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
