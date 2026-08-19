// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/DC1PalStateTreeTasks.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Controller/DC1PalAIController.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "AbilitySystemGlobals.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"

namespace DC1PalTaskHelper
{
	// 한 번에 달아나는 거리
	static constexpr float FleeStepDistance = 900.f;

	// 정면이 벽이면 조금씩 틀어가며 시도한다
	static constexpr float FleeFanAngles[] = { 0.f, 40.f, -40.f, 80.f, -80.f };

	static UDC1AbilitySystemComponent* GetASC(const AActor* Actor)
	{
		return Cast<UDC1AbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor));
	}

	static EStateTreeRunStatus ActivateAbility(UDC1AbilitySystemComponent& ASC, const FGameplayTag& AbilityTag, const FGameplayTag& StateTag, const AActor* Owner)
	{
		if (false == ASC.ActivateAbilityByTag(AbilityTag))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Pal] ActivateAbility: [%s] failed to activate [%s]. (Granted = %s)"),
				*GetNameSafe(Owner),
				*AbilityTag.ToString(),
				ASC.HasAbilityByTag(AbilityTag) ? TEXT("true") : TEXT("false"));

			return EStateTreeRunStatus::Failed;
		}

		if (false == ASC.HasMatchingGameplayTag(StateTag))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Pal] ActivateAbility: [%s] activated but [%s] is not applied. Check the ability's ActivationOwnedTags."),
				*AbilityTag.ToString(), *StateTag.ToString());

			return EStateTreeRunStatus::Succeeded;
		}

		return EStateTreeRunStatus::Running;
	}

	static EStateTreeRunStatus TickWhileActive(const UDC1AbilitySystemComponent& ASC, const FGameplayTag& StateTag)
	{
		if (ASC.HasMatchingGameplayTag(DC1PalTags::State_Pal_HitReacting))
		{
			return EStateTreeRunStatus::Running;
		}

		return ASC.HasMatchingGameplayTag(StateTag)
			? EStateTreeRunStatus::Running
			: EStateTreeRunStatus::Succeeded;
	}

	// 실패하면 갈 곳이 없다는 뜻
	static bool RequestFleeMove(ADC1PalAIController& AIController, const ADC1PalCharacter& Pal, const AActor& Target, float AcceptanceRadius)
	{
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(Pal.GetWorld());
		if (nullptr == NavSystem)
		{
			return false;
		}

		const FVector SelfLocation = Pal.GetActorLocation();
		const FVector TargetLocation = Target.GetActorLocation();
		const float CurrentDistanceSq = FVector::DistSquared2D(SelfLocation, TargetLocation);

		FVector AwayDirection = (SelfLocation - TargetLocation).GetSafeNormal2D();
		if (true == AwayDirection.IsNearlyZero())
		{
			AwayDirection = Pal.GetActorForwardVector().GetSafeNormal2D();
		}

		// 목표 지점이 NavMesh 밖이면 가장 가까운 NavMesh 지점으로 설정
		const FVector ProjectExtent(FleeStepDistance, FleeStepDistance, FleeStepDistance);

		for (const float Angle : FleeFanAngles)
		{
			const FVector Direction = AwayDirection.RotateAngleAxis(Angle, FVector::UpVector);
			const FVector IdealPoint = SelfLocation + (Direction * FleeStepDistance);

			FNavLocation Destination;
			if (false == NavSystem->ProjectPointToNavigation(IdealPoint, Destination, ProjectExtent))
			{
				continue;
			}

			if (FVector::DistSquared2D(Destination.Location, TargetLocation) <= CurrentDistanceSq)
			{
				continue;
			}

			if (EPathFollowingRequestResult::Failed != AIController.MoveToLocation(Destination.Location, AcceptanceRadius))
			{
				return true;
			}
		}

		return false;
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

	return DC1PalTaskHelper::ActivateAbility(*ASC, InstanceData.AbilityTag, InstanceData.ActiveStateTag, InstanceData.Actor);
}

EStateTreeRunStatus FDC1PalActivateAbilityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	const UDC1AbilitySystemComponent* ASC = DC1PalTaskHelper::GetASC(InstanceData.Actor);
	if (nullptr == ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	return DC1PalTaskHelper::TickWhileActive(*ASC, InstanceData.ActiveStateTag);
}

void FDC1PalActivateAbilityTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (UDC1AbilitySystemComponent* ASC = DC1PalTaskHelper::GetASC(InstanceData.Actor))
	{
		ASC->CancelAbilityByTag(InstanceData.AbilityTag);
	}
}


EStateTreeRunStatus FDC1PalActivateSkillTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.ActivatedSkillTag = FGameplayTag::EmptyTag;

	const ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return EStateTreeRunStatus::Failed;
	}

	const APawn* Pal = AIController->GetPawn();

	UDC1AbilitySystemComponent* ASC = DC1PalTaskHelper::GetASC(Pal);
	if (nullptr == ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	const FGameplayTag SkillTag = AIController->GetSelectedSkillTag();
	if (false == SkillTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] ActivateSkillTask: [%s] has no selected skill. Check PalData.SkillTags."), *GetNameSafe(Pal));
		return EStateTreeRunStatus::Failed;
	}

	const EStateTreeRunStatus Result = DC1PalTaskHelper::ActivateAbility(*ASC, SkillTag, DC1PalTags::State_Pal_Attacking, Pal);

	// 발동에 성공했으면 ExitState가 취소할 수 있게 기억해둔다
	if (EStateTreeRunStatus::Failed != Result)
	{
		InstanceData.ActivatedSkillTag = SkillTag;
	}

	return Result;
}

EStateTreeRunStatus FDC1PalActivateSkillTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	const ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return EStateTreeRunStatus::Failed;
	}

	const UDC1AbilitySystemComponent* ASC = DC1PalTaskHelper::GetASC(AIController->GetPawn());
	if (nullptr == ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	return DC1PalTaskHelper::TickWhileActive(*ASC, DC1PalTags::State_Pal_Attacking);
}

void FDC1PalActivateSkillTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (false == IsValid(InstanceData.AIController))
	{
		return;
	}

	if (true == InstanceData.ActivatedSkillTag.IsValid())
	{
		if (UDC1AbilitySystemComponent* ASC = DC1PalTaskHelper::GetASC(InstanceData.AIController->GetPawn()))
		{
			ASC->CancelAbilityByTag(InstanceData.ActivatedSkillTag);
		}
	}

	// 한 번 썼으면 다음엔 다시 고른다. 발동에 실패했어도 같은 스킬을 무한 재시도하지 않게 비운다
	InstanceData.AIController->ClearSelectedSkill();
}


EStateTreeRunStatus FDC1PalWanderTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return EStateTreeRunStatus::Failed;
	}

	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn());
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

	// 바라볼 타겟이 없으니 이동 방향으로 돌게 한다
	Pal->SetOrientToMovement(true);

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

	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn());
	const UDC1PalData* PalData = IsValid(Pal) ? Pal->GetPalData() : nullptr;
	if (nullptr == PalData)
	{
		return;
	}

	Pal->SetOrientToMovement(false);
	Pal->GetCharacterMovement()->MaxWalkSpeed = PalData->RunSpeed;
}


EStateTreeRunStatus FDC1PalFleeTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return EStateTreeRunStatus::Failed;
	}

	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn());
	const UDC1PalData* PalData = IsValid(Pal) ? Pal->GetPalData() : nullptr;
	const AActor* Target = AIController->GetCurrentTarget();
	if (nullptr == PalData || false == IsValid(Target))
	{
		return EStateTreeRunStatus::Failed;
	}

	Pal->SetOrientToMovement(true);
	DC1PalTaskHelper::RequestFleeMove(*AIController, *Pal, *Target, InstanceData.AcceptanceRadius);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FDC1PalFleeTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (EPathFollowingStatus::Idle != AIController->GetMoveStatus())
	{
		return EStateTreeRunStatus::Running;
	}

	// 도착
	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn());
	const UDC1PalData* PalData = IsValid(Pal) ? Pal->GetPalData() : nullptr;
	const AActor* Target = AIController->GetCurrentTarget();
	if (nullptr == PalData || false == IsValid(Target))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 다시 타겟 발견
	DC1PalTaskHelper::RequestFleeMove(*AIController, *Pal, *Target, InstanceData.AcceptanceRadius);

	return EStateTreeRunStatus::Running;
}

void FDC1PalFleeTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return;
	}

	AIController->StopMovement();

	if (ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn()))
	{
		Pal->SetOrientToMovement(false);
		AIController->SetHomeLocation(Pal->GetActorLocation());
	}
}

