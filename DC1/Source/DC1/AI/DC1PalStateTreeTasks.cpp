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
	static constexpr float FleeStepDistance = 1000.f;

	// 정면이 벽일 때 틀어볼 각도들
	static constexpr float FleeFanAngles[] = { 0.f, 40.f, -40.f, 80.f, -80.f };

	// 자리에 이만큼 붙으면 도착. 경로 요청에도 같은 값
	static constexpr float FollowStopRadius = 60.f;

	// 자리가 이만큼 움직여야 경로 재요청
	static constexpr float FollowRepathTolerance = 60.f;

	// 자리를 NavMesh로 당길 때 찾는 반경
	static constexpr float FollowProjectExtent = 300.f;

	// 이보다 느리면 주인이 서 있는 것으로 봄
	static constexpr float FollowOwnerMoveThreshold = 10.f;

	// 주인이 향하는 쪽. 서 있으면 보는 쪽
	static FVector GetOwnerHeading(const AActor& Owner)
	{
		const FVector Velocity = Owner.GetVelocity().GetSafeNormal2D();
		return (false == Velocity.IsNearlyZero()) ? Velocity : Owner.GetActorForwardVector().GetSafeNormal2D();
	}

	// 다음에 반드시 경로를 새로 잡도록 캐시도 비움
	static void StopFollowMove(AAIController& AIController, FVector& LastRequestedLocation)
	{
		LastRequestedLocation = FVector::ZeroVector;

		if (EPathFollowingStatus::Idle != AIController.GetMoveStatus())
		{
			AIController.StopMovement();
		}
	}

	// 낙오했을 때 주인 뒤로 복귀
	static void TeleportBehindOwner(ADC1PalAIController& AIController, ADC1PalCharacter& Pal, const FVector& BehindPoint, FVector& LastRequestedLocation)
	{
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(Pal.GetWorld());

		FNavLocation Behind;
		if (nullptr == NavSystem || false == NavSystem->ProjectPointToNavigation(BehindPoint, Behind, FVector(FollowProjectExtent)))
		{
			return;
		}

		StopFollowMove(AIController, LastRequestedLocation);
		Pal.SetActorLocation(Behind.Location + FVector(0.f, 0.f, Pal.GetSimpleCollisionHalfHeight()));

		UE_LOG(LogTemp, Log, TEXT("[Pal] %s teleported to owner (fell behind)."), *Pal.GetName());
	}

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
			UE_LOG(LogTemp, Warning, TEXT("[Pal] ActivateAbility: [%s] activated but [%s] is not applied. Check ActivationOwnedTags."),
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

	// 표적 반대쪽으로 StepDistance 만큼 물러날 지점을 잡아 이동시킨다.
	// 도주와 후퇴가 같은 일을 하고 거리만 달라서 함께 쓴다. 실패하면 갈 곳이 없다는 뜻
	static bool RequestFleeMove(ADC1PalAIController& AIController, const ADC1PalCharacter& Pal, const AActor& Target, float AcceptanceRadius, float StepDistance)
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

		const FVector ProjectExtent(StepDistance, StepDistance, StepDistance);

		for (const float Angle : FleeFanAngles)
		{
			const FVector Direction = AwayDirection.RotateAngleAxis(Angle, FVector::UpVector);
			const FVector IdealPoint = SelfLocation + (Direction * StepDistance);

			FNavLocation Destination;
			if (false == NavSystem->ProjectPointToNavigation(IdealPoint, Destination, ProjectExtent))
			{
				continue;
			}

			// 가까워지는 쪽은 도망이 아님
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

	// 성공했으면 ExitState가 취소할 수 있게 기억
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

	// 실패했어도 비움. 같은 스킬 무한 재시도 방지
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

	// 도착이든 중단이든 이동이 끝나면 배회 종료
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
	DC1PalTaskHelper::RequestFleeMove(*AIController, *Pal, *Target, InstanceData.AcceptanceRadius, DC1PalTaskHelper::FleeStepDistance);

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
	DC1PalTaskHelper::RequestFleeMove(*AIController, *Pal, *Target, InstanceData.AcceptanceRadius, DC1PalTaskHelper::FleeStepDistance);

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


EStateTreeRunStatus FDC1PalBackoffTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

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

	Pal->GetCharacterMovement()->MaxWalkSpeed = PalData->RunSpeed;

	// 벽에 몰린 상황
	if (false == DC1PalTaskHelper::RequestFleeMove(*AIController, *Pal, *Target, InstanceData.AcceptanceRadius, InstanceData.StepDistance))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FDC1PalBackoffTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 아직 물러나는 중
	if (EPathFollowingStatus::Idle != AIController->GetMoveStatus())
	{
		return EStateTreeRunStatus::Running;
	}

	// 더 가기
	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn());
	const AActor* Target = AIController->GetCurrentTarget();
	if (false == IsValid(Pal) || false == IsValid(Target))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (false == DC1PalTaskHelper::RequestFleeMove(*AIController, *Pal, *Target, InstanceData.AcceptanceRadius, InstanceData.StepDistance))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FDC1PalBackoffTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return;
	}

	AIController->StopMovement();

	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn());
	if (false == IsValid(Pal))
	{
		return;
	}

	Pal->SetOrientToMovement(false);

	if (const UDC1PalData* PalData = Pal->GetPalData())
	{
		Pal->GetCharacterMovement()->MaxWalkSpeed = PalData->RunSpeed;
	}
}


EStateTreeRunStatus FDC1PalFollowTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	InstanceData.LastRequestedLocation = FVector::ZeroVector;
	InstanceData.bCatchingUp = false;

	const ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return EStateTreeRunStatus::Failed;
	}

	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn());
	if (false == IsValid(Pal))
	{
		return EStateTreeRunStatus::Failed;
	}

	Pal->SetOrientToMovement(true);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FDC1PalFollowTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return EStateTreeRunStatus::Failed;
	}

	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn());
	AActor* FollowTarget = AIController->GetFollowTarget();
	const UDC1PalData* PalData = IsValid(Pal) ? Pal->GetPalData() : nullptr;
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(AIController->GetWorld());
	if (nullptr == PalData || false == IsValid(FollowTarget) || nullptr == NavSystem)
	{
		return EStateTreeRunStatus::Failed;
	}

	AIController->UpdateFollowAnchor(DeltaTime);

	const FVector PalLocation = Pal->GetActorLocation();
	const FVector TargetLocation = FollowTarget->GetActorLocation();

	// 설 자리. 식에 팰 위치가 없어야 팰이 움직여도 목표가 안 흔들림
	const FVector Goal = TargetLocation + (AIController->GetFollowAnchor() * InstanceData.AnchorDistance);

	FNavLocation Destination;
	if (false == NavSystem->ProjectPointToNavigation(Goal, Destination, FVector(DC1PalTaskHelper::FollowProjectExtent)))
	{
		return EStateTreeRunStatus::Running;
	}

	const float GoalDistance = FVector::Dist2D(PalLocation, Destination.Location);

	// 주인도 서 있고 자리에도 왔으면 추종 끝. 주시 상태로 넘긴다
	const bool bOwnerMoving = (FollowTarget->GetVelocity().SizeSquared2D() > FMath::Square(DC1PalTaskHelper::FollowOwnerMoveThreshold));
	if (false == bOwnerMoving && GoalDistance <= DC1PalTaskHelper::FollowStopRadius)
	{
		DC1PalTaskHelper::StopFollowMove(*AIController, InstanceData.LastRequestedLocation);

		return EStateTreeRunStatus::Succeeded;
	}

	// 지형에 걸렸거나 너무 뒤처진 경우
	if (FVector::DistSquared2D(PalLocation, TargetLocation) > FMath::Square(InstanceData.TeleportDistance))
	{
		const FVector BehindPoint = TargetLocation - (DC1PalTaskHelper::GetOwnerHeading(*FollowTarget) * InstanceData.AnchorDistance);
		DC1PalTaskHelper::TeleportBehindOwner(*AIController, *Pal, BehindPoint, InstanceData.LastRequestedLocation);

		return EStateTreeRunStatus::Running;
	}

	// 속도
	InstanceData.bCatchingUp = (true == InstanceData.bCatchingUp)
		? (GoalDistance > DC1PalTaskHelper::FollowStopRadius)
		: (GoalDistance > InstanceData.CatchUpDistance);

	const float OwnerSpeed = FollowTarget->GetVelocity().Size2D();
	Pal->GetCharacterMovement()->MaxWalkSpeed = (true == InstanceData.bCatchingUp)
		? FMath::Max(PalData->RunSpeed, OwnerSpeed * InstanceData.SpeedScale)
		: FMath::Max(PalData->WalkSpeed, OwnerSpeed);

	const bool bGoalMoved = FVector::DistSquared2D(Destination.Location, InstanceData.LastRequestedLocation) > FMath::Square(DC1PalTaskHelper::FollowRepathTolerance);
	if (true == bGoalMoved || EPathFollowingStatus::Idle == AIController->GetMoveStatus())
	{
		AIController->MoveToLocation(Destination.Location, DC1PalTaskHelper::FollowStopRadius, false);
		InstanceData.LastRequestedLocation = Destination.Location;
	}

	return EStateTreeRunStatus::Running;
}

void FDC1PalFollowTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return;
	}

	AIController->StopMovement();

	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn());
	if (false == IsValid(Pal))
	{
		return;
	}

	// 회전은 전투에서 조절
	Pal->SetOrientToMovement(false);

	// 추종용으로 올려둔 속도 복구
	if (const UDC1PalData* PalData = Pal->GetPalData())
	{
		Pal->GetCharacterMovement()->MaxWalkSpeed = PalData->RunSpeed;
	}
}


EStateTreeRunStatus FDC1PalFaceOwnerTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return EStateTreeRunStatus::Failed;
	}

	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn());
	AActor* FollowTarget = AIController->GetFollowTarget();
	if (false == IsValid(Pal) || false == IsValid(FollowTarget))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 회전은 focus가 구동
	Pal->SetOrientToMovement(false);
	Pal->GetCharacterMovement()->RotationRate.Yaw = InstanceData.TurnRate;
	AIController->SetFocus(FollowTarget, EAIFocusPriority::Gameplay);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FDC1PalFaceOwnerTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return EStateTreeRunStatus::Failed;
	}

	const AActor* FollowTarget = AIController->GetFollowTarget();
	if (false == IsValid(FollowTarget))
	{
		return EStateTreeRunStatus::Failed;
	}

	if (FollowTarget->GetVelocity().SizeSquared2D() > FMath::Square(DC1PalTaskHelper::FollowOwnerMoveThreshold))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return AIController->UpdateFollowAnchor(DeltaTime) ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}

void FDC1PalFaceOwnerTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return;
	}

	// 회전 조절
	if (AActor* Target = AIController->GetCurrentTarget())
	{
		AIController->SetFocus(Target, EAIFocusPriority::Gameplay);
	}
	else
	{
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn());
	if (false == IsValid(Pal))
	{
		return;
	}

	// orient켜지 않기. Follow면 EnterState가 켜고, 전투면 꺼야함

	if (const UDC1PalData* PalData = Pal->GetPalData())
	{
		Pal->GetCharacterMovement()->RotationRate.Yaw = PalData->RotationRateYaw;
	}
}
