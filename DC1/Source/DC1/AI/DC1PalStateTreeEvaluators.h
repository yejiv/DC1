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
	bool							bProvoked = { false };

	UPROPERTY(EditAnywhere, Category = Output)
	bool							bHitReacting = { false };

	UPROPERTY(EditAnywhere, Category = Output)
	bool							bOwnerInCombat = { false };	// 야생은 항상 false

	UPROPERTY(EditAnywhere, Category = Output)
	float							DistanceToTarget = { 0.f };

	UPROPERTY(EditAnywhere, Category = Output)
	float						SelectedMaxRange = { 0.f }; // Chase 진입 판단용

	UPROPERTY(EditAnywhere, Category = Output)
	float						SelectedApproachRange = { 0.f };	// Chase 종료 판단용
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
