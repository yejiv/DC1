// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Build/DC1BuildCheckComponent.h"

#include "Components/BoxComponent.h"


#include "Actor/BuildActor/DC1BuildActor.h"
#include "DrawDebugHelpers.h"
#include "Define/DC1Enums.h"

#include "Tags/Build/DC1GameplayTag_Build.h"


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

	if (false == IsSnap)
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

const FDC1BuildSnapPoint UDC1BuildCheckComponent::MakeSideSnapPoint(const FVector& InWorldNormal) const
{
	const FVector LocalNormal = GetComponentTransform().InverseTransformVectorNoScale(InWorldNormal);

	const float AbsX = FMath::Abs(LocalNormal.X);
	const float AbsY = FMath::Abs(LocalNormal.Y);

	const bool IsAxisX = AbsX > AbsY;

	const float Direction = IsAxisX ? LocalNormal.X : LocalNormal.Y;

	const float AxisSign = (Direction >= 0.f) ? 1.f : -1.f;

	return MakeSideSnapPointFromAxis(IsAxisX, AxisSign);
}

const FDC1BuildSnapPoint UDC1BuildCheckComponent::MakeNearestSideSnapPoint(const FVector& InWorldPoint) const
{
	const FVector LocalPoint = GetComponentTransform().InverseTransformPosition(InWorldPoint);
	const FVector CheckBoxExtent = GetUnscaledBoxExtent();

	float OverX = FMath::Abs(LocalPoint.X) - CheckBoxExtent.X;
	float OverY = FMath::Abs(LocalPoint.Y) - CheckBoxExtent.Y;

	const bool IsAxisX = OverX > OverY;

	const float Direction = IsAxisX ? LocalPoint.X : LocalPoint.Y;

	const float AxisSign = (Direction >= 0.f) ? 1.f : -1.f;

	return MakeSideSnapPointFromAxis(IsAxisX, AxisSign);
}

//void UDC1BuildCheckComponent::InitSnapPoints(TArray<FDC1BuildSnapPoint>& OutSnapPoints) const
//{
//	const AActor* Owner = GetOwner();
//	if (false == ::IsValid(Owner))
//	{
//		return;
//	}
//
//	OutSnapPoints.Reset();
//	OutSnapPoints.Reserve(SnapDatas.Num());
//
//	const FVector CheckBoxExtents = GetUnscaledBoxExtent();
//	const FTransform CheckBoxTransform = GetComponentTransform();
//
//	for (const FDC1BuildSnapData& SnapData : SnapDatas)
//	{
//		if (true == SnapData.SnapDirection.IsNearlyZero())
//		{
//			UE_LOG(LogTemp, Warning, TEXT("UDC1BuildCheckComponent :: Invalid SnapDirection : %s"), *Owner->GetName());
//			continue;
//		}
//
//		const FVector PointLocalLocation = (CheckBoxExtents * SnapData.SnapDirection);
//
//		const FVector PointWorldLocation = CheckBoxTransform.TransformPosition(PointLocalLocation);
//		const FVector PointWorldNomral = CheckBoxTransform.TransformVectorNoScale(PointLocalLocation).GetSafeNormal();
//
//		FDC1BuildSnapPoint SnapPoint;
//
//		SnapPoint.WorldLocation = PointWorldLocation;
//		SnapPoint.WorldNormal = PointWorldNomral;
//		SnapPoint.SnapType = SnapData.SnapType;
//		SnapPoint.AcceptTypes = SnapData.AcceptTypes;
//
//		OutSnapPoints.Add(SnapPoint);
//	}
//}

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

	bool IsAllHit = true;

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

			const bool IsHit = World->LineTraceSingleByChannel(Hit, Start, End, static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_BuildSurface), Params);
			bool CanBuildFromProbe = true;

			if (const ADC1BuildActor* HitBuildActor = Cast<ADC1BuildActor>(Hit.GetActor()))
			{
				if (false == HitBuildActor->GetIsBuildableSurface())
				{
					CanBuildFromProbe = false;
				}
			}

			const FVector ExpectedNormal = IsDown ? FVector::UpVector : (FVector::UpVector * -1.f);

			const float NormalDot = FMath::Clamp(FVector::DotProduct(Hit.ImpactNormal, ExpectedNormal), -1.f, 1.f);
			const float SurfaceAngle = FMath::RadiansToDegrees(FMath::Acos(NormalDot));
			CanBuildFromProbe = (IsHit && SurfaceAngle <= BuildProbeData.MaxSurfaceAngle);

			if (true == DrawDebugProbes)
			{
				FColor DebugColor = IsHit ? FColor::Green : FColor::Red;
				DrawDebugLine(World, Start, End, DebugColor);
			}

			if (false == CanBuildFromProbe)
			{
				if (false == DrawDebugProbes)
				{
					return false;
				}

				IsAllHit = false;
			}
		}
	}

	return IsAllHit;
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

const FDC1BuildSnapPoint UDC1BuildCheckComponent::MakeSideSnapPointFromAxis(bool InIsAxisX, float InAxisSign) const
{
	const FVector CheckBoxExtent = GetScaledBoxExtent();

	const FVector Axis = (InIsAxisX ? GetForwardVector() : GetRightVector()) * InAxisSign;

	const float AxisExtent = InIsAxisX ? CheckBoxExtent.X : CheckBoxExtent.Y;

	FDC1BuildSnapPoint SnapPoint;
	SnapPoint.WorldLocation = GetComponentLocation() + (Axis * AxisExtent);
	SnapPoint.WorldNormal = Axis;
	SnapPoint.SnapType = DC1BuildTags::Build_Snap_Side;
	SnapPoint.AcceptTypes.AddTag(DC1BuildTags::Build_Snap_Side);

	return SnapPoint;
}
