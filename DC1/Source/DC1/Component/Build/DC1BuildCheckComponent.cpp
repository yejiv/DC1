// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Build/DC1BuildCheckComponent.h"

#include "Components/BoxComponent.h"


#include "Actor/BuildActor/DC1BuildActor.h"
#include "DrawDebugHelpers.h"
#include "Define/DC1Structs.h"

#include "Tags/Build/DC1GameplayTag_Build.h"

#include "Engine/OverlapResult.h"
#include "CollisionShape.h"
#include "LandscapeProxy.h"

namespace 
{
	float ComputeBoxOverlapRatio(const FTransform& InA, const FVector& InExtentA, const FTransform& InB, const FVector& InExtentB)
	{
		const FVector LocalCenter = InA.InverseTransformPositionNoScale(InB.GetLocation());

		FVector Overlap = FVector::ZeroVector;

		for (int32 AxisIndex = 0; AxisIndex < 3; ++AxisIndex)
		{
			const FVector MyAxis = InA.GetUnitAxis(static_cast<EAxis::Type>(EAxis::X + AxisIndex));

			const FVector LocalAxis = InB.GetRotation().UnrotateVector(MyAxis);

			const float OtherRadius = (FMath::Abs(LocalAxis.X) * InExtentB.X) +
				(FMath::Abs(LocalAxis.Y) * InExtentB.Y) +
				(FMath::Abs(LocalAxis.Z) * InExtentB.Z);

			const float MyRadius = InExtentA[AxisIndex];

			const float AxisOverlap = (MyRadius + OtherRadius) - FMath::Abs(LocalCenter[AxisIndex]);

			Overlap[AxisIndex] = FMath::Clamp(AxisOverlap, 0.f, (2.f * FMath::Min(MyRadius, OtherRadius)));
		}

		const float OverlapVolume = Overlap.X * Overlap.Y * Overlap.Z;

		const float MyVolume = 8.f * InExtentA.X * InExtentA.Y * InExtentA.Z;
		const float OtherVolume = 8.f * InExtentB.X * InExtentB.Y * InExtentB.Z;

		const float MinVolume = FMath::Min(MyVolume, OtherVolume);

		return (MinVolume > KINDA_SMALL_NUMBER) ? (OverlapVolume / MinVolume) : 0.f;
	}

	bool GetOtherBox(const UPrimitiveComponent* InOtherComp, FDC1BuildBox& OutBox)
	{
		if (false == ::IsValid(InOtherComp))
		{
			return false;
		}

		const ADC1BuildActor* OtherBuild = Cast<ADC1BuildActor>(InOtherComp->GetOwner());
		if (true == ::IsValid(OtherBuild))
		{
			return OtherBuild->GetCheckBoxWorld(OutBox);
		}
		else
		{
			OutBox.Transform = FTransform(InOtherComp->Bounds.Origin);
			OutBox.Extent = InOtherComp->Bounds.BoxExtent;
		}

		return true;
	}
}

// Sets default values for this component's properties
UDC1BuildCheckComponent::UDC1BuildCheckComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

const float UDC1BuildCheckComponent::GetExtentCenterToDirection(const FVector& InWorldDirection, const FQuat& InQuat) const
{
	FVector CheckBoxExtents = GetScaledBoxExtent();

	const FQuat CheckBoxQuat = InQuat * GetRelativeRotation().Quaternion();
	const FVector LocalDirection = CheckBoxQuat.UnrotateVector(InWorldDirection.GetSafeNormal());

	float DirectionExtent = (FMath::Abs(LocalDirection.X) * CheckBoxExtents.X) +
		(FMath::Abs(LocalDirection.Y) * CheckBoxExtents.Y) +
		(FMath::Abs(LocalDirection.Z) * CheckBoxExtents.Z);

	return DirectionExtent;
}

bool UDC1BuildCheckComponent::CheckCanBuild()
{
	bool IsNoneOverlap = false;
	bool IsAttachedSurface = true;

	if (false == IsCheckSurface)
	{
		IsAttachedSurface = CheckSurface();
	}

	IsNoneOverlap = CheckOverlap();

	return (IsNoneOverlap && IsAttachedSurface);
}

const FTransform UDC1BuildCheckComponent::ComputePlaceTransform(const FHitResult& InHit, float InYaw) const
{
	const AActor* Owner = GetOwner();
	if (false == ::IsValid(Owner))
	{
		return FTransform::Identity;
	}

	FVector AlignAxis = FVector::UpVector;
	
	if (true == BuildProbeData.AlignToSurface)
	{
		const bool IsDown = (BuildProbeData.ProbeDirection == EDC1BuildProbeDirection::Down);

		const FVector ImpactNormal = FVector(InHit.ImpactNormal);

		AlignAxis = IsDown ? ImpactNormal : (ImpactNormal * -1.f);
	}


	FQuat AlignQuat = FQuat::FindBetweenNormals(FVector::UpVector, AlignAxis);
	FQuat FinalQuat = AlignQuat * (FRotator(0.f, InYaw, 0.f).Quaternion());

	FVector Location = InHit.ImpactPoint - FinalQuat.RotateVector(GetAttachLocalOffset());

	return FTransform(FinalQuat, Location, Owner->GetActorScale3D());
}

float UDC1BuildCheckComponent::ComputeOverlapRatio(const FTransform& InOtherTransform, const FVector& InOtherExtent) const
{
	const FTransform MyTransform = GetComponentTransform();
	const FVector MyExtent = GetScaledBoxExtent();

	return ComputeBoxOverlapRatio(MyTransform, MyExtent, InOtherTransform, InOtherExtent);
}

bool UDC1BuildCheckComponent::IsBlockedAt(const FTransform& InBoxTransform, const TArray<FDC1BuildBox>& InBlockers) const
{
	const FVector MyExtent = GetScaledBoxExtent();

	for (const FDC1BuildBox& Blocker : InBlockers)
	{
		const float OverlapRatio = ComputeBoxOverlapRatio(InBoxTransform, MyExtent, Blocker.Transform, Blocker.Extent);
		if (OverlapRatio > SnapOverlapAllowableRatio)
		{
			return true;
		}
	}

	return false;
}

void UDC1BuildCheckComponent::GatherSurfaceSupporters(TArray<ADC1BuildActor*>& OutSupporters, bool& OutIsOnGround) const
{
	OutSupporters.Reset();
	OutIsOnGround = false;

	ForEachProbe([this, &OutSupporters, &OutIsOnGround](const FHitResult& InHit, bool InIsHit)->bool
		{
			if (false == InIsHit)
			{
				return true;
			}

			ADC1BuildActor* HitBuildActor = Cast<ADC1BuildActor>(InHit.GetActor());
			if (false == ::IsValid(HitBuildActor) )
			{
				if (ComputeSurfaceAngle(InHit) <= BuildProbeData.MaxSurfaceAngle)
				{
					OutIsOnGround = true;
				}
				return true;
			}

			if (true == HitBuildActor->IsPreview())
			{
				return true;
			}

			if (true == HitBuildActor->GetIsBuildableSurface())
			{
				OutSupporters.AddUnique(HitBuildActor);
			}

			return true;
		});
}

bool UDC1BuildCheckComponent::CheckSurface()
{
	bool IsAllHit = true;

	const bool IsProbed = ForEachProbe([this, &IsAllHit](const FHitResult& InHit, bool InIsHit) -> bool 
		{
			bool CanBuildFromProbe = (InIsHit && ComputeSurfaceAngle(InHit) <= BuildProbeData.MaxSurfaceAngle);

			const ADC1BuildActor* HitBuildActor = Cast<ADC1BuildActor>(InHit.GetActor());
			if (true == ::IsValid(HitBuildActor))
			{
				CanBuildFromProbe &= HitBuildActor->GetIsBuildableSurface();
			}

			if (true == CanBuildFromProbe)
			{
				return true;
			}

			IsAllHit = false;

			return DrawDebugProbes;
		});

	if (false == IsProbed)
	{
		return false;
	}

	return IsAllHit;
}

bool UDC1BuildCheckComponent::CheckOverlap()
{
	const UWorld* World = GetWorld();
	const ADC1BuildActor* MyBuild = GetOwner<ADC1BuildActor>();
	if (false == ::IsValid(World) || false == ::IsValid(MyBuild))
	{
		return false;
	}

	TArray<FOverlapResult> Overlaps;
	FComponentQueryParams Params;
	Params.AddIgnoredActor(MyBuild);

	World->ComponentOverlapMulti(Overlaps, this, GetComponentLocation(), GetComponentQuat(), Params);

	const float AllowableRatio = MyBuild->GetIsAutoSnap() ? SnapOverlapAllowableRatio : FreeOverlapAllowableRatio;

	bool IsOnGround = false;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		const UPrimitiveComponent* OtherComp = Overlap.GetComponent();
		if (false == ::IsValid(OtherComp))
		{
			continue;
		}

		if (true == ::IsValid(Cast<ALandscapeProxy>(OtherComp->GetOwner())))
		{
			if (true == IsPillar)
			{
				IsOnGround = true;
				continue;
			}
			else
			{
				if (true == IsGroundPenetrated(OtherComp))
				{
					return false;
				}

				continue;
			}
		}

		FDC1BuildBox OtherBox;
		if (true == GetOtherBox(OtherComp, OtherBox))
		{
			float OverlapRatio = ComputeOverlapRatio(OtherBox.Transform, OtherBox.Extent);

			if (OverlapRatio > AllowableRatio)
			{
				return false;
			}
		}
	}

	return (false == IsPillar) || IsOnGround;
}

bool UDC1BuildCheckComponent::IsGroundPenetrated(const UPrimitiveComponent* InGroundComp) const
{
	// LandScape 판정 용 가상 위치로 이동 후 확인

	const float AllowableDepth = GetAllowableGroundDepth();

	const FVector RaisedLocation = GetComponentLocation() + (GetUpVector() * AllowableDepth);

	return InGroundComp->OverlapComponent(RaisedLocation, GetComponentQuat(), FCollisionShape::MakeBox(GetScaledBoxExtent()));
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

FVector UDC1BuildCheckComponent::GetAttachLocalOffset() const
{
	const AActor* Owner = GetOwner();
	if (false == ::IsValid(Owner))
	{
		return FVector::ZeroVector;
	}

	const bool IsDown = (EDC1BuildProbeDirection::Down == BuildProbeData.ProbeDirection);

	const float OffsetDirectionZ = IsDown ? -1.f : 1.f;

	const FVector WorldOffset = GetComponentLocation() + (GetUpVector() * GetScaledBoxExtent().Z * OffsetDirectionZ);

	// 회전 제거
	return Owner->GetActorQuat().UnrotateVector(WorldOffset - Owner->GetActorLocation());
}

bool UDC1BuildCheckComponent::ForEachProbe(TFunctionRef<bool(const FHitResult& InHit, bool InIsHit)> InOnProbe) const
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

	const float LocalZ = IsDown ? (CheckBoxExtent.Z * -1.f) : CheckBoxExtent.Z;

	const FVector TraceDirection = IsDown ? (GetUpVector() * -1.f) : (GetUpVector());

	const float StartOffset = (GetAllowableGroundDepth() + 2.f) * -1.f;

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
			const FVector End = (Start + (TraceDirection * (BuildProbeData.MaxTraceDistance + FMath::Abs(StartOffset))));

			FHitResult Hit;

			const bool IsHit = World->LineTraceSingleByChannel(Hit, Start, End, static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_BuildSurface), Params);

			if (true == DrawDebugProbes)
			{
				FColor DebugColor = IsHit ? FColor::Green : FColor::Red;
				DrawDebugLine(World, Start, End, DebugColor);
			}

			if (false == InOnProbe(Hit, IsHit))
			{
				return true;
			}
		}
	}

	return true;
}

float UDC1BuildCheckComponent::ComputeSurfaceAngle(const FHitResult& InHit) const
{
	const bool IsDown = (BuildProbeData.ProbeDirection == EDC1BuildProbeDirection::Down);
	const FVector ExpectedNormal = IsDown ? FVector::UpVector : FVector::DownVector;
	const float NormalDot = FMath::Clamp(FVector::DotProduct(InHit.ImpactNormal, ExpectedNormal), -1.f, 1.f);
	
	return FMath::RadiansToDegrees(FMath::Acos(NormalDot));
}

float UDC1BuildCheckComponent::GetAllowableGroundDepth() const
{
	return (GetScaledBoxExtent().Z * 2.f) * GroundContactRatio;
}
