// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Pal/DC1PalStateTreeConditions.h"

#include "Character/Pal/DC1PalAIController.h"

#include "StateTreeExecutionContext.h"

bool FDC1PalHasTargetCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	const bool bHasTarget = (nullptr != InstanceData.AIController)
		&& (nullptr != InstanceData.AIController->CurrentTarget);

	return (true == bInvert) ? !bHasTarget : bHasTarget;
}
