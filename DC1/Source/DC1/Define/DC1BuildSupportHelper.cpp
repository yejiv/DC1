#include "Define/DC1BuildSupportHelper.h"

#include "Actor/BuildActor/DC1BuildActor.h"
#include "Component/Build/DC1BuildLinkComponent.h"
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

	bool IsSocketLinked(const FDC1BuildSnapSocket& InSrc, const FDC1BuildSnapSocket& InDst)
	{
		const double DistSquared = FVector::DistSquared(InSrc.SocketTransform.GetLocation(), InDst.SocketTransform.GetLocation());
		if (DistSquared > FMath::Square(SocketLinkTolerance))
		{
			return false;
		}

		const double Dot = FVector::DotProduct(InSrc.SocketTransform.GetUnitAxis(EAxis::X), InDst.SocketTransform.GetUnitAxis(EAxis::X));
		if (Dot > SocketLinkOppositeDot)
		{
			return false;
		}

		if (false == InSrc.AcceptTypes.HasTagExact(InDst.SnapType) ||
			false == InDst.AcceptTypes.HasTagExact(InSrc.SnapType))
		{
			return false;
		}

		return true;
	}

	void QueryOverlapNeighbors(const ADC1BuildActor* InBuild, TArray<ADC1BuildActor*>& OutNeighbors)
	{
		if (false == ::IsValid(InBuild))
		{
			return;
		}

		FDC1BuildBox BuildBox;
		const UWorld* World = InBuild->GetWorld();
		if (false == ::IsValid(World) || false == InBuild->GetCheckBoxWorld(BuildBox))
		{
			return;
		}

		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AddObjectTypesToQuery(static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_Building));

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(InBuild);

		TArray<FOverlapResult> Overlaps;

		if (false == World->OverlapMultiByObjectType(Overlaps, BuildBox.Transform.GetLocation(), BuildBox.Transform.GetRotation(), ObjectParams, 
			FCollisionShape::MakeBox(BuildBox.Extent + FVector(NeighborQueryInflate)), Params))
		{
			return;
		}

		for (const FOverlapResult& Result : Overlaps)
		{
			ADC1BuildActor* OverlapActor = Cast<ADC1BuildActor>(Result.GetActor());
			if (false == ::IsValid(OverlapActor) || true == OverlapActor->IsPreview())
			{
				continue;
			}

			OutNeighbors.AddUnique(OverlapActor);
		}
	}

	// 범위 내에 지지하는 바닥이 있는지
	bool CanReachSupportingFloor(const ADC1BuildActor* InRoof)
	{
		FDC1BuildBox RoofBox;
		if (false == ::IsValid(InRoof) || false == InRoof->GetCheckBoxWorld(RoofBox))
		{
			return false;
		}

		int32 RoofSpan = InRoof->GetRoofSupportSpan();

		TSet<const UDC1BuildLinkComponent*> Visited;
		TArray<const UDC1BuildLinkComponent*> BFS;

		const UDC1BuildLinkComponent* Start = InRoof->GetLinkComponent();
		if (true == ::IsValid(Start))
		{
			Visited.Add(Start);
			BFS.Add(Start);
		}

		while (false == BFS.IsEmpty())
		{
			const UDC1BuildLinkComponent* Front = BFS.Pop();
			
			for (const FDC1BuildStructureLink& Link : Front->GetLinks())
			{
				if (true == Link.MySnapType.MatchesTagExact(DC1BuildTags::Build_Link_Surface))
				{
					continue;
				}

				const UDC1BuildLinkComponent* Other = Link.Other.Get();
				if (false == ::IsValid(Other) || Visited.Contains(Other))
				{
					continue;
				}
				
				const ADC1BuildActor* OtherActor = Other->GetOwner<ADC1BuildActor>();
				if (false == ::IsValid(OtherActor))
				{
					continue;
				}

				Visited.Add(Other);
				BFS.Push(Other);

				FDC1BuildBox FloorBox;
				if (EDC1BuildSupportRule::Ground != OtherActor->GetSupportRule() || false == OtherActor->GetCheckBoxWorld(FloorBox))
				{
					continue;
				}

				const FVector Delta = RoofBox.Transform.GetLocation() - FloorBox.Transform.GetLocation();
				int32 Dx = FMath::RoundToInt(FVector::DotProduct(Delta, FloorBox.Transform.GetUnitAxis(EAxis::X) / (FloorBox.Extent.X * 2.f)));
				int32 Dy = FMath::RoundToInt(FVector::DotProduct(Delta, FloorBox.Transform.GetUnitAxis(EAxis::Y) / (FloorBox.Extent.Y * 2.f)));

				if (FMath::Abs(Dx) <= RoofSpan && FMath::Abs(Dy) <= RoofSpan && false == (FMath::Abs(Dx) == RoofSpan && FMath::Abs(Dy) == RoofSpan))
				{
					return true;
				}
			}
		}

		return false;
	}

	// 내 타입을 가지는 Link가 존재하는지
	bool HasLinkOfType(const UDC1BuildLinkComponent* InLinkComp, const FGameplayTag& InType)
	{
		if (false == ::IsValid(InLinkComp))
		{
			return false;
		}

		for (const FDC1BuildStructureLink& Link : InLinkComp->GetLinks())
		{
			if (true == Link.MySnapType.MatchesTagExact(InType) && nullptr != Link.Other.Get())
			{
				return true;
			}
		}

		return false;
	}

	bool HasSurfaceSupporter(const ADC1BuildActor* InBuild, bool& OutIsGround)
	{
		OutIsGround = false;

		if (false == ::IsValid(InBuild))
		{
			return false;
		}

		TArray<ADC1BuildActor*> Supporters;

		if (false == InBuild->GatherSurfaceSupporters(Supporters, OutIsGround))
		{
			return false;
		}

		return false == Supporters.IsEmpty();
	}

	void BuildSocketLinks(const ADC1BuildActor* InBuild, TArray<FDC1BuildStructureLink>& OutLinks)
	{
		if (false == ::IsValid(InBuild))
		{
			return;
		}
		
		TArray<FDC1BuildSnapSocket> Sockets;
		if (false == InBuild->GetBuildSocketsToWorld(Sockets))
		{
			return;
		}

		TArray<ADC1BuildActor*> Neighbors;
		QueryOverlapNeighbors(InBuild, Neighbors);

		for (const ADC1BuildActor* Neighbor : Neighbors)
		{
			TArray<FDC1BuildSnapSocket> NeighborSockets;
			if (false == Neighbor->GetBuildSocketsToWorld(NeighborSockets))
			{
				continue;
			}

			for (const FDC1BuildSnapSocket& InSocket : Sockets)
			{
				for (const FDC1BuildSnapSocket& NeighborSocket : NeighborSockets)
				{
					if (true == IsSocketLinked(InSocket, NeighborSocket))
					{
						UDC1BuildLinkComponent* LinkComp = Neighbor->GetLinkComponent();
						if (true == ::IsValid(LinkComp))
						{
							OutLinks.Emplace(LinkComp, InSocket.SnapType, NeighborSocket.SnapType);
						}
					}
				}
			}
		}
	}

	void BuildSurfaceLinks(const ADC1BuildActor* InBuild, TArray<FDC1BuildStructureLink>& OutLinks)
	{
		if (false == ::IsValid(InBuild))
		{
			return;
		}

		bool IsOnGround = false;

		TArray<ADC1BuildActor*> Supporters;

		if (false == InBuild->GatherSurfaceSupporters(Supporters, IsOnGround))
		{
			return;
		}

		for (const ADC1BuildActor* Supporter : Supporters)
		{
			UDC1BuildLinkComponent* LinkComp = Supporter->GetLinkComponent();
			if (true == ::IsValid(LinkComp))
			{
				OutLinks.Emplace(LinkComp, DC1BuildTags::Build_Link_Surface, DC1BuildTags::Build_Link_Surface);
			}
		}
	}
}

void DC1BuildSupportHelper::BuildLinks(const ADC1BuildActor* InBuild, TArray<FDC1BuildStructureLink>& OutLinks)
{
	OutLinks.Reset();

	if (false == ::IsValid(InBuild))
	{
		return;
	}

	if (EDC1BuildSupportRule::Surface == InBuild->GetSupportRule())
	{
		BuildSurfaceLinks(InBuild, OutLinks);
	}
	else
	{
		BuildSocketLinks(InBuild, OutLinks);
	}
}

bool DC1BuildSupportHelper::HasSupport(ADC1BuildActor* InBuild)
{
	SCOPE_CYCLE_COUNTER(STAT_DC1_BuildHasSupport);

	if (false == ::IsValid(InBuild))
	{
		return false;
	}

	UDC1BuildLinkComponent* LinkComp = InBuild->GetLinkComponent();
	if (true == ::IsValid(LinkComp) && true == InBuild->IsPreview())
	{
		LinkComp->RefreshPreviewLinks();
	}

	switch (InBuild->GetSupportRule())
	{
	case EDC1BuildSupportRule::Ground:
	{
		return true;
	}
	case EDC1BuildSupportRule::Stack:
	{
		return HasLinkOfType(LinkComp, DC1BuildTags::Build_Snap_Attach_Wall);	
	}
	case EDC1BuildSupportRule::Surface :
	{
		bool IsOnGround = false;
		return HasSurfaceSupporter(InBuild, IsOnGround) || IsOnGround;
	}

	case EDC1BuildSupportRule::RoofSpan :
	{
		return CanReachSupportingFloor(InBuild);
	}
	}

	return false;
}

void DC1BuildSupportHelper::GatherNeighbors(const ADC1BuildActor* InBuild, TArray<ADC1BuildActor*>& OutNeighbors)
{
	OutNeighbors.Reset();

	if (false == ::IsValid(InBuild))
	{
		return;
	}

	const UDC1BuildLinkComponent* LinkComp = InBuild->GetLinkComponent();
	if (false == ::IsValid(LinkComp))
	{
		return;
	}

	for (const FDC1BuildStructureLink& Link : LinkComp->GetLinks())
	{
		UDC1BuildLinkComponent* OtherLink = Link.Other.Get();
		if (false == ::IsValid(OtherLink))
		{
			continue;
		}
		
		ADC1BuildActor* Neighbor = OtherLink->GetOwner<ADC1BuildActor>();
		if (true == ::IsValid(Neighbor))
		{
			OutNeighbors.AddUnique(Neighbor);
		}
	}
}