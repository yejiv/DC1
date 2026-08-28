// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/DC1PalStateTreeEvaluators.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Controller/DC1PalAIController.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "AbilitySystemComponent.h"
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
	InstanceData.bProvoked = false;
	InstanceData.bHitReacting = false;
	InstanceData.bOwnerInCombat = false;
	InstanceData.DistanceToTarget = TNumericLimits<float>::Max();
	InstanceData.SelectedMaxRange = 0.f;
	InstanceData.SelectedApproachRange = 0.f;
	InstanceData.SelectedMinRange = 0.f;

	ADC1PalAIController* AIController = InstanceData.AIController;
	if (false == ::IsValid(AIController))
	{
		return;
	}

	AIController->UpdateTarget();

	InstanceData.bProvoked = AIController->IsProvoked();
	InstanceData.bOwnerInCombat = (nullptr != AIController->GetFollowTargetEnemy());

	if (const ADC1PalCharacter* SelfPal = Cast<ADC1PalCharacter>(AIController->GetPawn()))
	{
		if (const UAbilitySystemComponent* ASC = SelfPal->GetAbilitySystemComponent())
		{
			InstanceData.bHitReacting = ASC->HasMatchingGameplayTag(DC1PalTags::State_Pal_HitReacting);
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

	// 스킬을 먼저 고르고, 거리 판단은 그 스킬의 사거리로 한다
	AIController->UpdateSelectedSkill();
	InstanceData.SelectedMaxRange = AIController->GetSelectedMaxRange();
	InstanceData.SelectedApproachRange = AIController->GetSelectedApproachRange();
	InstanceData.SelectedMinRange = AIController->GetSelectedMinRange();
}
