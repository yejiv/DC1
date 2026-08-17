// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/DC1PalStateTreeEvaluators.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Controller/DC1PalAIController.h"
#include "Data/DA/Pal/DC1PalData.h"

#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

namespace DC1PalTargetEvaluator
{
	static float GetSurfaceDistance2D(const AActor& Self, const AActor& Other)
	{
		const float CenterDistance = FVector::Dist2D(Self.GetActorLocation(), Other.GetActorLocation());
		const float SurfaceDistance = CenterDistance - Self.GetSimpleCollisionRadius() - Other.GetSimpleCollisionRadius();

		return FMath::Max(0.f, SurfaceDistance);
	}
}

void FDC1PalTargetEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	InstanceData.Target = nullptr;
	InstanceData.bHasTarget = false;
	InstanceData.DistanceToTarget = TNumericLimits<float>::Max();

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == ::IsValid(AIController))
	{
		return;
	}

	AIController->UpdateTarget();

	InstanceData.LastKnownLocation = AIController->GetLastKnownTargetLocation();

	// 사거리는 타겟 유무와 무관
	if (const ADC1PalCharacter* SelfPal = Cast<ADC1PalCharacter>(AIController->GetPawn()))
	{
		if (const UDC1PalData* PalData = SelfPal->GetPalData())
		{
			InstanceData.AttackRange = PalData->AttackRange;
			InstanceData.AttackExitRange = PalData->AttackExitRange;
		}
	}

	AActor* Target = AIController->GetCurrentTarget();
	const APawn* SelfPawn = AIController->GetPawn();

	if (true == ::IsValid(Target) && true == ::IsValid(SelfPawn))
	{
		InstanceData.Target = Target;
		InstanceData.bHasTarget = true;
		InstanceData.DistanceToTarget = DC1PalTargetEvaluator::GetSurfaceDistance2D(*SelfPawn, *Target);
	}
}
