// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Build/DC1BuildCheckComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UDC1BuildCheckComponent::UDC1BuildCheckComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UDC1BuildCheckComponent::CheckCanBuild()
{
	bool IsNoneOverlap = false;
	bool IsAttachedSurface = true;

	if (false == IsSnap)
	{
		IsAttachedSurface = CheckSurface();
	}
	
	IsNoneOverlap = CheckOverlap();

	return (IsNoneOverlap && IsAttachedSurface);
}

bool UDC1BuildCheckComponent::CheckSurface()
{
	if (BuildProbeData.ProbeCountX <= 0 || BuildProbeData.ProbeCountY <= 0 || BuildProbeData.MaxTraceDistance <= 0.f)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (false == ::IsValid(World))
	{
		return false;
	}

	const bool IsDown = (BuildProbeData.ProbeDirection == EDC1BuildProbeDirection::Down);

	const FVector CheckBoxExtent = GetUnscaledBoxExtent();
	const FTransform BoxTransform = GetComponentTransform();

	const float LocalZ = IsDown ? (CheckBoxExtent.Z * -1.f ) : CheckBoxExtent.Z;

	const FVector TraceDirection = IsDown ? (GetUpVector() * -1.f) : (GetUpVector());

	const float StartOffset = -2.f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	for (int32 x = 0; x < BuildProbeData.ProbeCountX; ++x)
	{
		const float LocalX = ComputeProbeAxisOffset(x, BuildProbeData.ProbeCountX, CheckBoxExtent.X);

		for (int32 y = 0; y < BuildProbeData.ProbeCountY; ++y)
		{
			const float LocalY = ComputeProbeAxisOffset(y, BuildProbeData.ProbeCountY, CheckBoxExtent.Y);
			
			const FVector ProbeWorld = BoxTransform.TransformPosition(FVector(LocalX, LocalY, LocalZ));
			
			const FVector Start = (ProbeWorld + (TraceDirection * StartOffset));
			const FVector End = (Start + (TraceDirection * ( BuildProbeData.MaxTraceDistance + FMath::Abs(StartOffset))));

			FHitResult Hit;

			const bool IsHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

			if (true == DrawDebugProbes)
			{
				FColor DebugColor = IsHit ? FColor::Green : FColor::Red;
				DrawDebugLine(World, Start, End, DebugColor);
			}

			if (false == IsHit)
			{
				if (false == DrawDebugProbes)
				{
					return false;
				}
			}
		}
	}

	return true;
}

bool UDC1BuildCheckComponent::CheckOverlap()
{
	TArray<AActor*> OverlapActors;

	this->GetOverlappingActors(OverlapActors);

	for (AActor* OverlapActor : OverlapActors)
	{
		if (OverlapActor == GetOwner())
		{
			continue;
		}

		return false;
	}

	return true;
}

float UDC1BuildCheckComponent::ComputeProbeAxisOffset(int32 InIndex, int32 InCount, float InExtent) const
{
	if (InCount <= 1)
	{
		return 0.f;
	}

	const float Ratio = (static_cast<float>(InIndex) / static_cast<float>(InCount - 1));

	return (FMath::Lerp((InExtent * -1.f), InExtent, Ratio) * BuildProbeData.ProbeDistanceFromCorner);
}
