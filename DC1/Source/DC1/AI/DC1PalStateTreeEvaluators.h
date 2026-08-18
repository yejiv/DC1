// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "StateTreeEvaluatorBase.h"
#include "DC1PalStateTreeEvaluators.generated.h"

class ADC1PalAIController;

USTRUCT()
struct FDC1PalTargetEvaluatorInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ADC1PalAIController>	AIController = { nullptr };

	UPROPERTY(EditAnywhere, Category = Output)
	TObjectPtr<AActor>				Target = { nullptr };

	UPROPERTY(EditAnywhere, Category = Output)
	bool							bHasTarget = { false };

	UPROPERTY(EditAnywhere, Category = Output)
	float							DistanceToTarget = { 0.f };

	// 전이 조건이 DistanceToTarget과 비교할 값. 종마다 달라서 PalData로 판단
	UPROPERTY(EditAnywhere, Category = Output)
	float							AttackRange = { 0.f };

	UPROPERTY(EditAnywhere, Category = Output)
	float							AttackExitRange = { 0.f };
};


USTRUCT(DisplayName = "Pal Target", Category = "Pal")
struct FDC1PalTargetEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FDC1PalTargetEvaluatorInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
