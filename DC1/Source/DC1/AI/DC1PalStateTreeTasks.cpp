// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/DC1PalStateTreeTasks.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Controller/DC1PalAIController.h"
#include "Data/DA/Pal/DC1PalData.h"

#include "AbilitySystemGlobals.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
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


EStateTreeRunStatus FDC1PalWanderTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return EStateTreeRunStatus::Failed;
	}

	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn());
	const UDC1PalData* PalData = IsValid(Pal) ? Pal->GetPalData() : nullptr;
	if (nullptr == PalData)
	{
		return EStateTreeRunStatus::Failed;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(Pal->GetWorld());
	if (nullptr == NavSystem)
	{
		return EStateTreeRunStatus::Failed;
	}

	FNavLocation Destination;
	if (false == NavSystem->GetRandomReachablePointInRadius(AIController->GetHomeLocation(), PalData->WanderRadius, Destination))
	{
		return EStateTreeRunStatus::Failed;
	}

	Pal->GetCharacterMovement()->MaxWalkSpeed = PalData->WalkSpeed;

	AIController->SetFocalPoint(Destination.Location, EAIFocusPriority::Move);

	const EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(Destination.Location, InstanceData.AcceptanceRadius);

	return (EPathFollowingRequestResult::Failed == Result)
		? EStateTreeRunStatus::Failed
		: EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FDC1PalWanderTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	const ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 도착이든 중단이든 이동이 끝나면 배회 끝인 걸로
	return (EPathFollowingStatus::Idle == AIController->GetMoveStatus())
		? EStateTreeRunStatus::Succeeded
		: EStateTreeRunStatus::Running;
}

void FDC1PalWanderTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return;
	}

	AIController->StopMovement();
	AIController->ClearFocus(EAIFocusPriority::Move);

	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn());
	const UDC1PalData* PalData = IsValid(Pal) ? Pal->GetPalData() : nullptr;
	if (nullptr == PalData)
	{
		return;
	}

	Pal->GetCharacterMovement()->MaxWalkSpeed = PalData->RunSpeed;
}

