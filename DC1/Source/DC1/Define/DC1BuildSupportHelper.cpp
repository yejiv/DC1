#include "Define/DC1BuildSupportHelper.h"

#include "Actor/BuildActor/DC1BuildActor.h"
#include "Define/DC1Enums.h"
#include "Define/DC1Structs.h"
#include "Define/DC1Stats.h"
#include "Tags/Build/DC1GameplayTag_Build.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "CollisionShape.h"

namespace
{
	// 스냅 Margin 여유 포함 연결되어있다고 판정할 거리
	constexpr float SocketLinkTolerance = 5.f;

	// 짝 소켓은 X축이 반대 이기에 -1.f 근사치
	constexpr float SocketLinkOppositeDot = -0.9f;

	// 체크 박스를 부풀릴 수치 ( 접촉 판정 여유 )
	// 맞닿은 박스를 후보로 잡는다
	constexpr float NeighborQueryInflate = 10.f;

	// 지붕 경계에서의 여유
	constexpr float RoofGapEpsilon = 5.f;

	struct FDC1SupportCache
	{
		TMap<const ADC1BuildActor*, TArray<ADC1BuildActor*>>		Neighbors;
		TMap<const ADC1BuildActor*, TArray<FDC1BuildSnapSocket>>	Sockets;
	};

	TArray<ADC1BuildActor*> GetNeighbors(const ADC1BuildActor* InBuild, FDC1SupportCache& InOutCache)
	{
		const TArray<ADC1BuildActor*>* Find = InOutCache.Neighbors.Find(InBuild);
		if (nullptr != Find)
		{
			return *Find;
		}

		TArray<ADC1BuildActor*> Neighbors;
		
		FDC1BuildBox Box;
		UWorld* World = InBuild->GetWorld();

		if (false == InBuild->GetCheckBoxWorld(Box) || false == ::IsValid(World))
		{
			InOutCache.Neighbors.Add(InBuild, Neighbors);

			return Neighbors;
		}

		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AddObjectTypesToQuery(static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_Building));

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(InBuild);

		TArray<FOverlapResult> Overlaps;

		if (true == World->OverlapMultiByObjectType(Overlaps, Box.Transform.GetLocation(), Box.Transform.GetRotation(), ObjectParams,
			FCollisionShape::MakeBox(Box.Extent + FVector(NeighborQueryInflate)), Params))
		{
			for (const FOverlapResult& Result : Overlaps)
			{
				ADC1BuildActor* OverlapActor = Cast<ADC1BuildActor>(Result.GetActor());
				if (false == ::IsValid(OverlapActor) || true == OverlapActor->IsPreview())
				{
					continue;
				}

				Neighbors.AddUnique(OverlapActor);
			}
		}

		InOutCache.Neighbors.Add(InBuild, Neighbors);

		return Neighbors;
	}

	void EnsureSockets(const ADC1BuildActor* InBuild, FDC1SupportCache& InOutCache)
	{
		if (nullptr != InOutCache.Sockets.Find(InBuild))
		{
			return;
		}

		TArray<FDC1BuildSnapSocket> Sockets;
		InBuild->GetBuildSocketsToWorld(Sockets);
		InOutCache.Sockets.Add(InBuild, Sockets);
	}

	bool IsSocketLinked(const FDC1BuildSnapSocket& InMySocket, const FDC1BuildSnapSocket& InOtherSocket)
	{
		float DistSquared = FVector::DistSquared(InMySocket.SocketTransform.GetLocation(), InOtherSocket.SocketTransform.GetLocation());
		float AttachDot = FVector::DotProduct(InMySocket.SocketTransform.GetUnitAxis(EAxis::X), InOtherSocket.SocketTransform.GetUnitAxis(EAxis::X));

		bool IsNear = DistSquared <= FMath::Square(SocketLinkTolerance);
	
		bool IsAttach = AttachDot <= SocketLinkOppositeDot;

		bool IsMatching = InMySocket.AcceptTypes.HasTagExact(InOtherSocket.SnapType) || InOtherSocket.AcceptTypes.HasTagExact(InMySocket.SnapType);

		return (IsNear && IsAttach && IsMatching);
	}

	bool HasSocketLink(const ADC1BuildActor* InMyActor, const FGameplayTag& InMySnapType, const ADC1BuildActor* InOtherActor, FDC1SupportCache& InOutCache)
	{
		EnsureSockets(InMyActor, InOutCache);
		EnsureSockets(InOtherActor, InOutCache);

		const TArray<FDC1BuildSnapSocket>& MySockets = InOutCache.Sockets.FindChecked(InMyActor);
		const TArray<FDC1BuildSnapSocket>& OtherSockets = InOutCache.Sockets.FindChecked(InOtherActor);

		for (const FDC1BuildSnapSocket& MySocket : MySockets)
		{
			if (InMySnapType != MySocket.SnapType)
			{
				continue;
			}
			
			for (const FDC1BuildSnapSocket& OtherSocket : OtherSockets)
			{
				if (true == IsSocketLinked(MySocket, OtherSocket))
				{
					return true;
				}
			}
		}
		
		return false;
	}

	float ComputeAxisGap(const FDC1BuildBox& InA, const FDC1BuildBox& InB, const FVector& InAxis)
	{
		const float CenterDistance = FMath::Abs(FVector::DotProduct(InA.Transform.GetLocation() - InB.Transform.GetLocation(), InAxis));
		
		const FVector LocalAxisA = InA.Transform.GetRotation().UnrotateVector(InAxis);
		const FVector LocalAxisB = InB.Transform.GetRotation().UnrotateVector(InAxis);

		const float RadiusA = (FMath::Abs(LocalAxisA.X) * InA.Extent.X) +
			(FMath::Abs(LocalAxisA.Y) * InA.Extent.Y) +
			(FMath::Abs(LocalAxisA.Z) * InA.Extent.Z);

		const float RadiusB = (FMath::Abs(LocalAxisB.X) * InB.Extent.X) +
			(FMath::Abs(LocalAxisB.Y) * InB.Extent.Y) +
			(FMath::Abs(LocalAxisB.Z) * InB.Extent.Z);

		return FMath::Max(0.f, CenterDistance - RadiusA - RadiusB);
	}

	bool IsSupportedByStack(const ADC1BuildActor* InBuild, const TSet<ADC1BuildActor*>& InExcluded, FDC1SupportCache& InOutCache)
	{
		// 벽 - 소켓 or Probe 중 하나
		
		// 소켓 확인
		const TArray<ADC1BuildActor*> Neighbors = GetNeighbors(InBuild, InOutCache);
		
		for (const ADC1BuildActor* Neighbor : Neighbors)
		{
			if (false == InExcluded.Contains(Neighbor) && true == HasSocketLink(InBuild, DC1BuildTags::Build_Snap_Attach_Wall, Neighbor, InOutCache))
			{
				return true;
			}
		}

		// Probe 확인
		TArray<ADC1BuildActor*> SurfaceActors;
		bool IsOnGround = false;
		InBuild->GatherSurfaceSupporters(SurfaceActors, IsOnGround);

		for (const ADC1BuildActor* SurfaceActor : SurfaceActors)
		{
			if (false == InExcluded.Contains(SurfaceActor))
			{
				return true;
			}
		}

		return false;
	}

	bool IsSupportedBySurface(const ADC1BuildActor* InBuild, const TSet<ADC1BuildActor*>& InExcluded)
	{
		TArray<ADC1BuildActor*> SurfaceActors;
		bool IsOnGround = false;
		InBuild->GatherSurfaceSupporters(SurfaceActors, IsOnGround);

		if (true == IsOnGround)
		{
			return true;
		}

		for (const ADC1BuildActor* SurfaceActor : SurfaceActors)
		{
			if (false == InExcluded.Contains(SurfaceActor))
			{
				return true;
			}
		}

		return false;
	}

	void GatherRoofGroup(ADC1BuildActor* InRoof, const TSet<ADC1BuildActor*>& InExcluded, FDC1SupportCache& InOutCache, TSet<ADC1BuildActor*>& OutGroup)
	{
		// Side.Roof 소켓으로 이어진 지붕들 ( InRoof 포함 )
		OutGroup.Add(InRoof);
		
		TArray<ADC1BuildActor*> Pending;
		Pending.Add(InRoof);

		while (false == Pending.IsEmpty())
		{
			ADC1BuildActor* Front = Pending.Pop();

			TArray<ADC1BuildActor*> Neighbors = GetNeighbors(Front, InOutCache);
			
			for (ADC1BuildActor* Neighbor : Neighbors)
			{
				if (true == InExcluded.Contains(Neighbor) || EDC1BuildSupportRule::RoofSpan != Neighbor->GetSupportRule() || true == OutGroup.Contains(Neighbor))
				{
					continue;
				}

				if (true == HasSocketLink(Front, DC1BuildTags::Build_Snap_Side_Roof, Neighbor, InOutCache))
				{
					OutGroup.Add(Neighbor);
					Pending.Add(Neighbor);
				}
			}
		}
	}

	void GatherWallGroup(const TSet<ADC1BuildActor*>& InRoofGroup, const TSet<ADC1BuildActor*>& InExcluded, FDC1SupportCache& InOutCache, TArray<const ADC1BuildActor*>& OutWalls)
	{
		for (const ADC1BuildActor* Roof : InRoofGroup)
		{
			TArray<ADC1BuildActor*> Neighbors = GetNeighbors(Roof, InOutCache);

			for (ADC1BuildActor* Neighbor : Neighbors)
			{
				if (true == InExcluded.Contains(Neighbor))
				{
					continue;
				}

				if (true == HasSocketLink(Roof, DC1BuildTags::Build_Snap_Attach_Roof, Neighbor, InOutCache))
				{
					OutWalls.AddUnique(Neighbor);
				}
			}
		}
	}

	bool IsSupportedByRoofSpan(ADC1BuildActor* InRoof, const TSet<ADC1BuildActor*>& InExcluded, FDC1SupportCache& InOutCache)
	{
		// 지붕 축별 범위
		TSet<ADC1BuildActor*> RoofGroup;
		GatherRoofGroup(InRoof, InExcluded, InOutCache, RoofGroup);

		TArray<const ADC1BuildActor*> WallGroup;
		GatherWallGroup(RoofGroup, InExcluded, InOutCache, WallGroup);

		if (true == RoofGroup.IsEmpty() || true == WallGroup.IsEmpty())
		{
			return false;
		}

		FDC1BuildBox RoofBox;
		if (false == InRoof->GetCheckBoxWorld(RoofBox))
		{
			return false;
		}

		const FVector AxisX = RoofBox.Transform.GetUnitAxis(EAxis::X);
		const FVector AxisY = RoofBox.Transform.GetUnitAxis(EAxis::Y);

		const FVector2D RoofSize = FVector2D(RoofBox.Extent.X * 2.f, RoofBox.Extent.Y * 2.f);

		float MinGapX = FLT_MAX;
		float MinGapY = FLT_MAX;

		for (const ADC1BuildActor* Wall : WallGroup)
		{
			FDC1BuildBox WallBox;
			if (false == Wall->GetCheckBoxWorld(WallBox))
			{
				continue;
			}

			const float GapX = ComputeAxisGap(RoofBox, WallBox, AxisX);
			const float GapY = ComputeAxisGap(RoofBox, WallBox, AxisY);

			MinGapX = FMath::Min(GapX, MinGapX);
			MinGapY = FMath::Min(GapY, MinGapY);
		}

		const FVector2D ToleranceGap = RoofSize * (InRoof->GetRoofSupportSpan() - 1) + RoofGapEpsilon;

		return MinGapX <= ToleranceGap.X && MinGapY <= ToleranceGap.Y;
	}

	bool IsSupported(ADC1BuildActor* InBuild, const TSet<ADC1BuildActor*>& InExcluded, FDC1SupportCache& InOutCache)
	{
		switch (InBuild->GetSupportRule())
		{
		case EDC1BuildSupportRule::Ground :
			return true;
		case EDC1BuildSupportRule::Surface :
			return IsSupportedBySurface(InBuild, InExcluded);
		case EDC1BuildSupportRule::Stack :
			return IsSupportedByStack(InBuild, InExcluded, InOutCache);
		case EDC1BuildSupportRule::RoofSpan :
			return IsSupportedByRoofSpan(InBuild, InExcluded, InOutCache);
		}

		return false;
	}

	void GatherDependents(const ADC1BuildActor* InBuild, const TSet<ADC1BuildActor*>& InExcluded, FDC1SupportCache& InOutCache, TArray<ADC1BuildActor*>& OutDependents)
	{
		TArray<ADC1BuildActor*> Neighbors = GetNeighbors(InBuild, InOutCache);

		for (ADC1BuildActor* Neighbor : Neighbors)
		{
			EDC1BuildSupportRule SupportRule = Neighbor->GetSupportRule();
			if (true == InExcluded.Contains(Neighbor) || EDC1BuildSupportRule::Ground == SupportRule)
			{
				continue;
			}
			
			if (EDC1BuildSupportRule::RoofSpan == SupportRule)
			{
				TSet<ADC1BuildActor*> RoofGroup;
				GatherRoofGroup(Neighbor, InExcluded, InOutCache, RoofGroup);

				for (ADC1BuildActor* Roof : RoofGroup)
				{
					OutDependents.AddUnique(Roof);
				}
			}
			OutDependents.AddUnique(Neighbor);
		}
	}
}


bool DC1BuildSupportHelper::HasSupport(ADC1BuildActor* InBuild)
{
	if (false == ::IsValid(InBuild))
	{
		return false;
	}

	TSet<ADC1BuildActor*> Excluded;
	FDC1SupportCache Cache;

	return IsSupported(InBuild, Excluded, Cache);
}

void DC1BuildSupportHelper::CollectCascade(ADC1BuildActor* InOrigin, TSet<ADC1BuildActor*>& OutDoomed)
{
	SCOPE_CYCLE_COUNTER(STAT_DC1_BuildCollectCascade);

	OutDoomed.Reset();

	if (false == ::IsValid(InOrigin))
	{
		return;
	}

	OutDoomed.Add(InOrigin);

	FDC1SupportCache Cache;
	TArray<ADC1BuildActor*> Dependents;

	GatherDependents(InOrigin, OutDoomed, Cache, Dependents);

	while (false == Dependents.IsEmpty())
	{
		ADC1BuildActor* Target = Dependents.Pop();

		if (true == OutDoomed.Contains(Target) || true == IsSupported(Target, OutDoomed, Cache))
		{
			continue;
		}

		OutDoomed.Add(Target);
		GatherDependents(Target, OutDoomed, Cache, Dependents);
	}
}
