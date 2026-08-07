// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "StateTreeConditionBase.h"
#include "DC1PalStateTreeConditions.generated.h"

class ADC1PalAIController;

USTRUCT()
struct FDC1PalHasTargetConditionInstanceData
{
	GENERATED_BODY()

	// Context 카테고리는 컴파일 시 스키마의 AIController 컨텍스트에 자동으로 묶인다.
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<ADC1PalAIController> AIController = { nullptr };
};

/**
 * Pal이 추격 대상을 들고 있는지 검사한다.
 */
USTRUCT(DisplayName = "Pal Has Target", Category = "Pal")
struct FDC1PalHasTargetCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FDC1PalHasTargetConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bInvert = { false };
};
