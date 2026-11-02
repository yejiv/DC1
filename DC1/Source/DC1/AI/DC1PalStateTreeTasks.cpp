// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/DC1PalStateTreeTasks.h"

#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Controller/DC1PalAIController.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Interface/DC1WorkTargetInterface.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"
#include "Tags/State/DC1GameplayTag_State.h"

#include "AbilitySystemGlobals.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"

namespace DC1PalTaskHelper
{
	// 정면이 벽일 때 틀어볼 각도들
	static constexpr float FleeFanAngles[] = { 0.f, 40.f, -40.f, 80.f, -80.f };

	// 순간이동으로 되돌릴 때 주인 뒤로 두는 거리
	static constexpr float TeleportBehindDistance = 400.f;

	// 자리에 이만큼 붙으면 도착. 경로 요청에도 같은 값
	static constexpr float FollowStopRadius = 60.f;

	// 자리가 이만큼 움직여야 경로 재요청
	static constexpr float FollowRepathTolerance = 60.f;

	// 자리를 NavMesh로 당길 때 찾는 반경
	static constexpr float FollowProjectExtent = 300.f;

	// 이보다 느리면 주인이 서 있는 것으로 봄
	static constexpr float FollowOwnerMoveThreshold = 10.f;

	// 거점 팰 배회 지점이 거점 밖으로 나오면 다시 뽑는 횟수. 다 실패하면 배회 중심으로
	static constexpr int32 BaseWanderMaxAttempts = 8;

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
	// BaseAngle: 정면 뒤에서 좌우로 틀 기준 각도. 0이면 예전과 같은 직선 뒤
	static bool RequestFleeMove(ADC1PalAIController& AIController, const ADC1PalCharacter& Pal, const AActor& Target, float AcceptanceRadius, float StepDistance, float BaseAngle = 0.f)
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

		if (false == FMath::IsNearlyZero(BaseAngle))
		{
			AwayDirection = AwayDirection.RotateAngleAxis(BaseAngle, FVector::UpVector);
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

	ADC1PalAIController* AIController = InstanceData.AIController;
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

	// 스킬 태그 발동한다. (쿨타임은 어빌리티가 커밋할 때)
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
	if (false == NavSystem->GetRandomReachablePointInRadius(AIController->GetHomeLocation(), AIController->GetWanderRadius(), Destination))
	{
		return EStateTreeRunStatus::Failed;
	}

	// 거점 팰은 거점 범위 안에서만 움직인다.
	// 배회 중심(HomeLocation)은 팰박스 근처 내비메시 점이라 팰박스와 어긋난 만큼 반경이 범위 밖으로 밀린다
	if (const ADC1PalBoxActor* PalBox = Pal->GetBasePalBox())
	{
		if (false == PalBox->IsInBase(Pal->GetActorLocation()))
		{
			// 이미 밖이면 배회 대신 중심으로 돌아온다
			Destination.Location = AIController->GetHomeLocation();
		}
		else
		{
			int32 Attempt = 0;
			while (false == PalBox->IsInBase(Destination.Location))
			{
				if (++Attempt >= DC1PalTaskHelper::BaseWanderMaxAttempts
					|| false == NavSystem->GetRandomReachablePointInRadius(AIController->GetHomeLocation(), AIController->GetWanderRadius(), Destination))
				{
					Destination.Location = AIController->GetHomeLocation();
					break;
				}
			}
		}
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


EStateTreeRunStatus FDC1PalRetreatTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		// 십중팔구 ST 애셋에서 AIController 바인딩이 비었다.
		// 여기서 실패하면 상태가 통째로 실패해 Root 로 튕기고, 트리가 엉뚱하게 돈다
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Retreat: AIController is not bound. Check the Pal Retreat node in the StateTree asset."));
		return EStateTreeRunStatus::Failed;
	}

	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn());
	const UDC1PalData* PalData = IsValid(Pal) ? Pal->GetPalData() : nullptr;
	if (nullptr == PalData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Retreat: %s has no PalData."), *GetNameSafe(AIController->GetPawn()));
		return EStateTreeRunStatus::Failed;
	}

	// 물러날 상대가 없다. 이건 오류가 아니라 할 일이 없는 것이다.
	// Failed 로 끝내면 상태가 실패해 Root 로 튕기고, 트리가 같은 자리를 무한히 돈다
	AActor* Target = AIController->GetCurrentTarget();
	if (false == IsValid(Target))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	Pal->SetOrientToMovement(true);
	Pal->GetCharacterMovement()->MaxWalkSpeed = PalData->FleeSpeed;

	// 등을 돌리는 순간 시야에서 사라져 표적 참조가 끊긴다. 지금 붙들어둔다
	InstanceData.Threat = Target;

	// 이번 물러남의 방향. 진입당 한 번만 굴린다. 매 틱 굴리면 좌우로 떨기만 한다
	const float LateralAngle = FMath::FRandRange(-InstanceData.LateralAngleMax, InstanceData.LateralAngleMax);

	// 갈 곳이 없다. 벽에 몰린 상황
	if (false == DC1PalTaskHelper::RequestFleeMove(*AIController, *Pal, *Target, InstanceData.AcceptanceRadius, InstanceData.StepDistance, LateralAngle))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FDC1PalRetreatTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
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

	// 한 걸음으로 끝낸다. 더 물러날지는 트리가 다시 고른다
	if (false == InstanceData.bRepeat)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(AIController->GetPawn());

	// 지각이 아니라 진입 때 붙들어둔 위협을 본다. 등을 돌린 사이 표적 기억이 지워져도 계속 달아난다
	const AActor* Threat = InstanceData.Threat.Get();
	if (false == IsValid(Pal) || false == IsValid(Threat))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 충분히 멀어졌다
	if (FVector::Dist2D(Pal->GetActorLocation(), Threat->GetActorLocation()) >= InstanceData.SafeDistance)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (false == DC1PalTaskHelper::RequestFleeMove(*AIController, *Pal, *Threat, InstanceData.AcceptanceRadius, InstanceData.StepDistance))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FDC1PalRetreatTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == IsValid(AIController))
	{
		return;
	}

	AIController->StopMovement();

	if (false == InstanceData.bRepeat)
	{
		AIController->NotifyBackoffPerformed();
	}

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
	const FVector Goal = TargetLocation + AIController->GetFollowAnchor();

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
		// 순간이동은 앵커와 무관하게 주인 바로 뒤로. 앵커 길이를 그대로 쓰면 멀리 떨어진 자리로 튄다
		const FVector BehindPoint = TargetLocation - (DC1PalTaskHelper::GetOwnerHeading(*FollowTarget) * DC1PalTaskHelper::TeleportBehindDistance);
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

	// 주인 속도가 아니라 자리 오차로 본다. 목표는 주인을 따라 움직이므로
	// "주인이 얼마나 움직였나" 와 같은 값이고, 기준점을 저장할 필요가 없다
	const APawn* Pal = AIController->GetPawn();
	if (false == IsValid(Pal))
	{
		return EStateTreeRunStatus::Failed;
	}

	const FVector Goal = FollowTarget->GetActorLocation() + AIController->GetFollowAnchor();
	if (FVector::Dist2D(Pal->GetActorLocation(), Goal) > InstanceData.FollowDeadzone)
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


namespace DC1PalTaskHelper
{
	// 표적이 정면에서 InAcceptAngle 안인가 (수평)
	static bool IsFacing(const AActor& Pal, const AActor& Target, float InAcceptAngle)
	{
		const FVector ToTarget = (Target.GetActorLocation() - Pal.GetActorLocation()).GetSafeNormal2D();
		if (true == ToTarget.IsNearlyZero())
		{
			return true;
		}

		const FVector Forward = Pal.GetActorForwardVector().GetSafeNormal2D();
		return FVector::DotProduct(Forward, ToTarget) >= FMath::Cos(FMath::DegreesToRadians(InAcceptAngle));
	}
}

EStateTreeRunStatus FDC1PalFaceTargetTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.ElapsedTime = 0.f;

	ADC1PalAIController* AIController = InstanceData.AIController;
	ADC1PalCharacter* Pal = IsValid(AIController) ? Cast<ADC1PalCharacter>(AIController->GetPawn()) : nullptr;
	AActor* Target = IsValid(AIController) ? AIController->GetCurrentTarget() : nullptr;
	if (false == IsValid(Pal) || false == IsValid(Target))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 회전은 focus가 구동
	Pal->SetOrientToMovement(false);
	if (Target != AIController->GetFocusActor())
	{
		AIController->SetFocus(Target, EAIFocusPriority::Gameplay);
	}

	return DC1PalTaskHelper::IsFacing(*Pal, *Target, InstanceData.AcceptAngle) ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FDC1PalFaceTargetTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.ElapsedTime += DeltaTime;

	ADC1PalAIController* AIController = InstanceData.AIController;
	const APawn* Pal = IsValid(AIController) ? AIController->GetPawn() : nullptr;
	const AActor* Target = IsValid(AIController) ? AIController->GetCurrentTarget() : nullptr;
	if (false == IsValid(Pal) || false == IsValid(Target))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (InstanceData.ElapsedTime >= InstanceData.GiveUpTime)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return DC1PalTaskHelper::IsFacing(*Pal, *Target, InstanceData.AcceptAngle) ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
}


namespace DC1PalTaskHelper
{
	static IDC1WorkTargetInterface* AssignNearestWorkTarget(ADC1PalCharacter& Pal, const ADC1PalBoxActor& PalBox)
	{
		TArray<TPair<float, IDC1WorkTargetInterface*>> Candidates;

		for (const TWeakInterfacePtr<IDC1WorkTargetInterface>& Weak : PalBox.GetWorkTargets())
		{
			IDC1WorkTargetInterface* Target = Weak.Get();
			const AActor* TargetActor = Cast<AActor>(Weak.GetObject());
			if (nullptr == Target || nullptr == TargetActor)
			{
				continue;
			}

			if (false == DC1WorkTarget::CanPalWorkAt(&Pal, Target->GetWorkTypeTag(), TargetActor->GetActorLocation()))
			{
				continue;
			}

			Candidates.Add({ FVector::DistSquared2D(Pal.GetActorLocation(), TargetActor->GetActorLocation()), Target });
		}

		Candidates.Sort([](const TPair<float, IDC1WorkTargetInterface*>& A, const TPair<float, IDC1WorkTargetInterface*>& B)
			{
				return A.Key < B.Key;
			});

		for (const TPair<float, IDC1WorkTargetInterface*>& Candidate : Candidates)
		{
			if (true == Candidate.Value->AssignPalWorker(&Pal))
			{
				return Candidate.Value;
			}
		}

		return nullptr;
	}

	static bool IsNearWorkTarget(const AActor& Pal, const AActor& TargetActor, float InAcceptanceRadius)
	{
		const float SurfaceDistance = FVector::Dist2D(Pal.GetActorLocation(), TargetActor.GetActorLocation())
			- Pal.GetSimpleCollisionRadius() - TargetActor.GetSimpleCollisionRadius();

		return SurfaceDistance <= InAcceptanceRadius;
	}
}

EStateTreeRunStatus FDC1PalWorkTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bArrived = false;

	ADC1PalAIController* AIController = InstanceData.AIController;
	ADC1PalCharacter* Pal = IsValid(AIController) ? Cast<ADC1PalCharacter>(AIController->GetPawn()) : nullptr;
	const ADC1PalBoxActor* PalBox = IsValid(Pal) ? Pal->GetBasePalBox() : nullptr;
	if (nullptr == PalBox)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 할 일이 없음
	IDC1WorkTargetInterface* Target = DC1PalTaskHelper::AssignNearestWorkTarget(*Pal, *PalBox);
	AActor* TargetActor = Cast<AActor>(Target);
	if (nullptr == TargetActor)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	Pal->SetWorkTarget(Target);
	Pal->SetOrientToMovement(true);

	if (EPathFollowingRequestResult::Failed == AIController->MoveToActor(TargetActor, InstanceData.AcceptanceRadius))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FDC1PalWorkTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	ADC1PalCharacter* Pal = IsValid(AIController) ? Cast<ADC1PalCharacter>(AIController->GetPawn()) : nullptr;
	if (false == IsValid(Pal))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	// 배정이 풀림. 다시 고름
	IDC1WorkTargetInterface* Target = Pal->GetCurrentWorkTarget();
	AActor* TargetActor = Cast<AActor>(Target);
	if (nullptr == TargetActor || false == Target->IsPalWorkerAssigned(Pal))
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (false == InstanceData.bArrived)
	{
		if (EPathFollowingStatus::Idle != AIController->GetMoveStatus())
		{
			return EStateTreeRunStatus::Running;
		}

		// 이동 끝, 도착X
		if (false == DC1PalTaskHelper::IsNearWorkTarget(*Pal, *TargetActor, InstanceData.AcceptanceRadius))
		{
			return EStateTreeRunStatus::Succeeded;
		}

		InstanceData.bArrived = true;
		Pal->SetOrientToMovement(false);
		AIController->SetFocus(TargetActor, EAIFocusPriority::Gameplay);
	}

	UDC1AbilitySystemComponent* ASC = DC1PalTaskHelper::GetASC(Pal);
	if (nullptr == ASC)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (true == ASC->HasMatchingGameplayTag(DC1StateTags::State_Working))
	{
		return EStateTreeRunStatus::Running;
	}

	const EStateTreeRunStatus Result = DC1PalTaskHelper::ActivateAbility(*ASC, DC1PalTags::Ability_Pal_Work, DC1StateTags::State_Working, Pal);
	return (EStateTreeRunStatus::Running == Result) ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

void FDC1PalWorkTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	ADC1PalAIController* AIController = InstanceData.AIController;
	ADC1PalCharacter* Pal = IsValid(AIController) ? Cast<ADC1PalCharacter>(AIController->GetPawn()) : nullptr;
	if (false == IsValid(Pal))
	{
		return;
	}

	AIController->StopMovement();
	AIController->ClearFocus(EAIFocusPriority::Gameplay);
	Pal->SetOrientToMovement(false);

	if (UDC1AbilitySystemComponent* ASC = DC1PalTaskHelper::GetASC(Pal))
	{
		ASC->CancelAbilityByTag(DC1PalTags::Ability_Pal_Work);
	}

	IDC1WorkTargetInterface* Target = Pal->GetCurrentWorkTarget();
	if (nullptr != Target && true == Target->IsPalWorkerAssigned(Pal))
	{
		Target->UnassignPalWorker(Pal);
	}

	Pal->SetWorkTarget(nullptr);
}
