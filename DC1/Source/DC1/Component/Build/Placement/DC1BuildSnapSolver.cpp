// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Build/Placement/DC1BuildSnapSolver.h"

#include "Actor/BuildActor/DC1BuildActor.h"

#include "Define/DC1Enums.h"
#include "Define/DC1Stats.h"

#include "GameFramework/Pawn.h"

#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "CollisionShape.h"

namespace
{
	const FTransform SnapSocketFlip = FTransform(FRotator(0.f, 180.f, 0.f));
}

bool UDC1BuildSnapSolver::FindSnapTransform(const FDC1BuildSnapInput& InInput, FTransform& OutTransform) const
{
	if (false == ::IsValid(InInput.Preview))
	{
		return false;
	}

	FDC1BuildSnapPair SnapPair;
	if (false == FindSnapPair(InInput, SnapPair))
	{
		return false;
	}

	OutTransform = MakeSnapTransform(InInput.Preview, InInput.CurrentYaw, SnapPair);

	return true;
}

bool UDC1BuildSnapSolver::BeginManualSnap(const FDC1BuildSnapInput& InInput)
{
	if (false == ::IsValid(InInput.Preview))
	{
		return false;
	}

	FDC1BuildSnapPair SnapPair;
	if (false == FindSnapPair(InInput, SnapPair))
	{
		return false;
	}

	SnapSocketLocationWorld = SnapPair.TargetSocket.SocketTransform.GetLocation();
	SnapSocketNormalWorld = SnapPair.TargetSocket.SocketTransform.GetUnitAxis(EAxis::X);
	SnapBaseYaw = SnapPair.TargetActor->GetActorRotation().Yaw;

	const FQuat SnapQuat = FRotator(0.f, SnapBaseYaw, 0.f).Quaternion();
	SnapMinDistance = InInput.Preview->GetExtentCenterToDirection(SnapSocketNormalWorld, SnapQuat) + SnapContactMargin;

	SnapTarget = SnapPair.TargetActor;

	IsSnapping = true;

	return true;
}

void UDC1BuildSnapSolver::ClearSnap()
{
	IsSnapping = false;
	SnapTarget = nullptr;
	SnapSocketLocationWorld = FVector::ZeroVector;
	SnapSocketNormalWorld = FVector::ZeroVector;
}

void UDC1BuildSnapSolver::OnPlacementConfirmed(const ADC1BuildActor* InPreview)
{
	if (false == IsSnapping || false == ::IsValid(InPreview))
	{
		return;
	}

	const FQuat SnapQuat = FRotator(0.f, SnapBaseYaw, 0.f).Quaternion();
	SnapMinDistance = InPreview->GetExtentCenterToDirection(SnapSocketNormalWorld, SnapQuat);

	const FVector NewBoxCenter = InPreview->GetCheckBoxCenter();
	const float HalfExtent = SnapMinDistance + SnapContactMargin;

	SnapSocketLocationWorld = NewBoxCenter + (SnapSocketNormalWorld * HalfExtent);
}

const FVector UDC1BuildSnapSolver::GetSnappedPlacePoint(const FVector& InAimPoint) const
{
	if (false == IsSnapping)
	{
		return InAimPoint;
	}

	const float SnapDot = FVector::DotProduct(InAimPoint - SnapSocketLocationWorld, SnapSocketNormalWorld);

	const float Distance = FMath::Max(SnapDot, SnapMinDistance);

	return SnapSocketLocationWorld + (SnapSocketNormalWorld * Distance);
}

bool UDC1BuildSnapSolver::FindSnapPair(const FDC1BuildSnapInput& InInput, FDC1BuildSnapPair& OutPair) const
{
	SCOPE_CYCLE_COUNTER(STAT_DC1_FindsnapPair);

	if (false == ::IsValid(InInput.Pawn) || false == ::IsValid(InInput.Preview))
	{
		return false;
	}

	TArray<FDC1BuildSnapSocket> PreviewSockets;
	if (false == InInput.Preview->GetBuildSocketsToWorld(PreviewSockets))
	{
		return false;
	}

	TArray<FDC1BuildBox> Blockers;

	TSet<ADC1BuildActor*> RayCandidates;
	FVector RayPoint;

	if (true == FindSnapCandidateByRay(InInput, RayCandidates, RayPoint))
	{
		TSet<ADC1BuildActor*> Neighbors;
		FindSnapCandidates(InInput, RayPoint, Neighbors);

		GatherBlockers(Neighbors, Blockers);

		if (true == FindFreeSnapPair(InInput, RayCandidates, RayPoint, PreviewSockets, Blockers, OutPair))
		{
			return true;
		}
	}

	TSet<ADC1BuildActor*> Candidates;
	FindSnapCandidates(InInput, InInput.AimPoint, Candidates);

	GatherBlockers(Candidates, Blockers);

	return FindFreeSnapPair(InInput, Candidates, InInput.AimPoint, PreviewSockets, Blockers, OutPair);
}

bool UDC1BuildSnapSolver::FindSnapCandidateByRay(const FDC1BuildSnapInput& InInput, TSet<ADC1BuildActor*>& OutCandidates, FVector& OutAimPoint) const
{
	OutCandidates.Reset();

	if (false == ::IsValid(InInput.RayTarget))
	{
		return false;
	}

	if (false == InInput.RayTarget->TraceCheckBox(InInput.RayStart, InInput.RayEnd, FCollisionQueryParams::DefaultQueryParam, InInput.AimInflate, OutAimPoint))
	{
		// 박스가 메시보다 작은 경우에 월드 Hit로 대체
		OutAimPoint = InInput.RayImpactPoint;
	}

	OutCandidates.Add(InInput.RayTarget);

	return true;
}

void UDC1BuildSnapSolver::FindSnapCandidates(const FDC1BuildSnapInput& InInput, const FVector& InAimPoint, TSet<ADC1BuildActor*>& OutCandidates) const
{
	OutCandidates.Reset();

	SCOPE_CYCLE_COUNTER(STAT_DC1_SnapOverlapQuery);

	const UWorld* World = GetWorld();
	if (false == ::IsValid(World))
	{
		return;
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_Building));

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(InInput.Pawn);
	Params.AddIgnoredActor(InInput.Preview);

	TArray<FOverlapResult> Overlaps;

	if (false == World->OverlapMultiByObjectType(Overlaps, InAimPoint, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(InInput.SearchRadius), Params))
	{
		return;
	}

	for (const FOverlapResult& Result : Overlaps)
	{
		ADC1BuildActor* OverlapActor = Cast<ADC1BuildActor>(Result.GetActor());
		if (false == ::IsValid(OverlapActor))
		{
			continue;
		}

		OutCandidates.Add(OverlapActor);
	}
}

void UDC1BuildSnapSolver::GatherBlockers(const TSet<ADC1BuildActor*>& InNeighbors, TArray<FDC1BuildBox>& OutBlockers) const
{
	OutBlockers.Reset();

	for (const ADC1BuildActor* Neighbor : InNeighbors)
	{
		if (true == ::IsValid(Neighbor))
		{
			FDC1BuildBox NeighborBox;
			if (true == Neighbor->GetCheckBoxWorld(NeighborBox))
			{
				OutBlockers.Add(NeighborBox);
			}
		}
	}
}

bool UDC1BuildSnapSolver::FindFreeSnapPair(const FDC1BuildSnapInput& InInput, const TSet<ADC1BuildActor*>& InCandidates, const FVector& InAimPoint, const TArray<FDC1BuildSnapSocket>& InPreviewSockets, const TArray<FDC1BuildBox>& InBlockers, FDC1BuildSnapPair& OutPair) const
{
	if (false == ::IsValid(InInput.Preview))
	{
		return false;
	}

	// 검사는 뽑힌 소켓만 - 모든 소켓을 돌며 매번 검사하면 후보 * 소켓 * 프리뷰 * 이웃으로 곱해짐 ( 측정 0.85ms )
	TArray<TWeakObjectPtr<const UDC1BuildSocketComponent>> ExcludedSockets;

	// 사방이 찬 경우의 상한선
	constexpr int32 MaxTries = 8;

	for (int32 Try = 0; Try < MaxTries; ++Try)
	{
		FDC1BuildSnapPair Pair;

		if (false == FindBestSnapPair(InInput, InCandidates, InAimPoint, InPreviewSockets, ExcludedSockets, Pair))
		{
			return false;
		}

		{
			SCOPE_CYCLE_COUNTER(STAT_DC1_SnapBlockTest);

			if (false == InInput.Preview->IsBlockedAt(MakeSnapTransform(InInput.Preview, InInput.CurrentYaw, Pair), InBlockers))
			{
				OutPair = Pair;
				return true;
			}
		}

		ExcludedSockets.Add(Pair.TargetSocket.SourceSocket);
	}

	return false;
}

bool UDC1BuildSnapSolver::FindBestSnapPair(const FDC1BuildSnapInput& InInput, const TSet<ADC1BuildActor*>& InCandidates, const FVector& InAimPoint, const TArray<FDC1BuildSnapSocket>& InPreviewSockets, const TArray<TWeakObjectPtr<const UDC1BuildSocketComponent>>& InExcludedSockets, FDC1BuildSnapPair& OutPair) const
{
	if (false == ::IsValid(InInput.Pawn))
	{
		return false;
	}

	const FVector FacingPoint = InInput.Pawn->GetActorLocation();

	float BestActorDistSquared = FLT_MAX;

	bool IsFind = false;

	for (const ADC1BuildActor* Candidate : InCandidates)
	{
		if (true == Candidate->IsPreview())
		{
			continue;
		}

		const float ActorDistSquared = FVector::DistSquared(InAimPoint, Candidate->GetCheckBoxCenter());

		if (ActorDistSquared >= BestActorDistSquared)
		{
			continue;
		}

		TArray<FDC1BuildSnapSocket> TargetSockets;
		if (false == Candidate->GetBuildSocketsToWorld(TargetSockets))
		{
			continue;
		}

		float BestScore = FLT_MAX * -1.f;
		float BestDistSquared = FLT_MAX;
		float BestFacing = FLT_MAX * -1.f;

		for (const FDC1BuildSnapSocket& TargetSocket : TargetSockets)
		{
			// 이번 프레임에 이미 판정된 소켓
			if (true == InExcludedSockets.Contains(TargetSocket.SourceSocket))
			{
				continue;
			}

			float Score = 0.f;
			float DistSquared = 0.f;
			float Facing = 0.f;

			TargetSocket.EvaluateFrom(InAimPoint, FacingPoint, Score, DistSquared, Facing);

			if (false == FDC1BuildSnapSocket::IsBetter(Score, DistSquared, Facing, BestScore, BestDistSquared, BestFacing))
			{
				continue;
			}

			FDC1BuildSnapSocket PreviewSocket;
			if (false == SelectPreviewSocket(InInput.Preview, TargetSocket, InPreviewSockets, InInput.ControlYaw, PreviewSocket))
			{
				continue;
			}

			BestScore = Score;
			BestDistSquared = DistSquared;
			BestFacing = Facing;

			BestActorDistSquared = ActorDistSquared;

			FDC1BuildSnapPair SnapPair;
			SnapPair.TargetActor = Candidate;
			SnapPair.TargetSocket = TargetSocket;
			SnapPair.PreviewSocket = PreviewSocket;

			OutPair = SnapPair;

			IsFind = true;
		}
	}

	return IsFind;
}

bool UDC1BuildSnapSolver::SelectPreviewSocket(const ADC1BuildActor* InPreview, const FDC1BuildSnapSocket& InTarget, const TArray<FDC1BuildSnapSocket>& InPreviewSockets, float InReferenceYaw, FDC1BuildSnapSocket& OutPreview) const
{
	float MinDifference = FLT_MAX;

	bool IsFindPreviewSocket = false;
	for (const FDC1BuildSnapSocket& PreviewSocket : InPreviewSockets)
	{
		if (false == InTarget.AcceptTypes.HasTagExact(PreviewSocket.SnapType))
		{
			continue;
		}

		const FTransform Desired = SnapSocketFlip * InTarget.SocketTransform;

		const FQuat DeltaQuat = Desired.GetRotation() * PreviewSocket.SocketTransform.GetRotation().Inverse();

		const float ResultYaw = (DeltaQuat * InPreview->GetActorQuat()).Rotator().Yaw;

		const float Difference = FMath::Abs(FMath::FindDeltaAngleDegrees(ResultYaw, InReferenceYaw));

		if (Difference >= MinDifference)
		{
			continue;
		}

		MinDifference = Difference;

		OutPreview = PreviewSocket;

		IsFindPreviewSocket = true;
	}

	return IsFindPreviewSocket;
}

const FTransform UDC1BuildSnapSolver::MakeSnapTransform(const ADC1BuildActor* InPreview, float InCurrentYaw, const FDC1BuildSnapPair& InPair) const
{
	// 두 소켓이 마주보게
	const FTransform Desired = SnapSocketFlip * InPair.TargetSocket.SocketTransform;

	const FQuat DeltaQuat = Desired.GetRotation() * InPair.PreviewSocket.SocketTransform.GetRotation().Inverse();

	const FVector ActorLocation = InPreview->GetActorLocation();
	const FVector SocketOffset = InPair.PreviewSocket.SocketTransform.GetLocation() - ActorLocation;

	const FVector AttachAxis = InPair.TargetSocket.SocketTransform.GetUnitAxis(EAxis::X);
	const FVector Anchor = Desired.GetLocation() + (AttachAxis * SnapContactMargin);

	const FQuat BaseQuat = DeltaQuat * InPreview->GetActorQuat();
	const FVector BaseLocation = Anchor - DeltaQuat.RotateVector(SocketOffset);

	// 피벗 ( 박스 중심 )
	const FVector BoxOffset = InPreview->GetCheckBoxCenter() - ActorLocation;
	const FVector PivotWorld = BaseLocation + DeltaQuat.RotateVector(BoxOffset);

	const float SnappedYaw = FMath::GridSnap(InCurrentYaw, InPreview->GetSnappedRotationStep());
	const FQuat ExtraQuat = FRotator(0.f, SnappedYaw, 0.f).Quaternion();

	const FQuat FinalQuat = ExtraQuat * BaseQuat;
	const FVector FinalLocation = PivotWorld + ExtraQuat.RotateVector(BaseLocation - PivotWorld);

	return FTransform(FinalQuat, FinalLocation, InPreview->GetActorScale3D());
}
