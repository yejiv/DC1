// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/DC1PalStateTreeTasks.h"

#include "Component/DC1AbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "GameFramework/Actor.h"
#include "StateTreeExecutionContext.h"

namespace DC1PalTaskHelper
{
	static UDC1AbilitySystemComponent* GetASC(const AActor* Actor)
	{
		return Cast<UDC1AbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor));
	}
}

EStateTreeRunStatus FDC1PalActivateAbilityTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	UDC1AbilitySystemComponent* ASC = DC1PalTaskHelper::GetASC(InstanceData.Actor);
	if (nullptr == ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] ActivateAbilityTask: No ASC on [%s]."), *GetNameSafe(InstanceData.Actor));
		return EStateTreeRunStatus::Failed;
	}

	if (false == ASC->ActivateAbilityByTag(InstanceData.AbilityTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] ActivateAbilityTask: [%s] failed to activate [%s]. (Granted = %s)"),
			*GetNameSafe(InstanceData.Actor),
			*InstanceData.AbilityTag.ToString(),
			ASC->HasAbilityByTag(InstanceData.AbilityTag) ? TEXT("true") : TEXT("false"));

		return EStateTreeRunStatus::Failed;
	}

	if (false == ASC->HasMatchingGameplayTag(InstanceData.ActiveStateTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] ActivateAbilityTask: [%s] activated but [%s] is not applied. Check the ability's ActivationOwnedTags."),
			*InstanceData.AbilityTag.ToString(), *InstanceData.ActiveStateTag.ToString());
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FDC1PalActivateAbilityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	const UDC1AbilitySystemComponent* ASC = DC1PalTaskHelper::GetASC(InstanceData.Actor);
	if (nullptr == ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	return ASC->HasMatchingGameplayTag(InstanceData.ActiveStateTag)
		? EStateTreeRunStatus::Running
		: EStateTreeRunStatus::Succeeded;
}

void FDC1PalActivateAbilityTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (UDC1AbilitySystemComponent* ASC = DC1PalTaskHelper::GetASC(InstanceData.Actor))
	{
		ASC->CancelAbilityByTag(InstanceData.AbilityTag);
	}
}

