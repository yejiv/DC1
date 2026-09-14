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
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UDC1BuildCheckComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled(DrawDebugSocket);
}

void UDC1BuildCheckComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DrawDebugSockets();
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

bool UDC1BuildCheckComponent::GetBuildSocketsWorld(TArray<FDC1BuildSnapSocket>& OutSockets) const
{
	OutSockets.Reset();
	OutSockets.Reserve(SnapDatas.Num());

	const FTransform CheckBoxTransform = GetComponentTransform();

	for (const FDC1BuildSnapSocketData& SnapData : SnapDatas)
	{
		if (false == SnapData.SnapType.IsValid())
		{
			continue;
		}

		FDC1BuildSnapSocket SnapSocket;

		// UE는 자식 * 부모
		SnapSocket.SocketTransform = MakeSnapSocketLocalTransform(SnapData) * CheckBoxTransform;

		SnapSocket.SnapType = SnapData.SnapType;
		SnapSocket.AcceptTypes = SnapData.AcceptTypes;

		OutSockets.Add(SnapSocket);
	}

	if (OutSockets.IsEmpty())
	{
		return false;
	}

	return true;
}

float UDC1BuildCheckComponent::ComputeOverlapRatio(const FTransform& InOtherTransform, const FVector& InOtherExtent) const
{
	const FTransform MyTransform = GetComponentTransform();
	const FVector MyExtent = GetScaledBoxExtent();

	const FVector LocalCenter = MyTransform.InverseTransformPosition(InOtherTransform.GetLocation());

	FVector Overlap = FVector::ZeroVector;

	for (int32 AxisIndex = 0; AxisIndex < 3; ++AxisIndex)
	{
		const FVector MyAxis = MyTransform.GetUnitAxis(static_cast<EAxis::Type>(EAxis::X + AxisIndex));

		const FVector LocalAxis = InOtherTransform.GetRotation().UnrotateVector(MyAxis);

		const float OtherRadius = (FMath::Abs(LocalAxis.X) * InOtherExtent.X) +
			(FMath::Abs(LocalAxis.Y) * InOtherExtent.Y) +
			(FMath::Abs(LocalAxis.Z) * InOtherExtent.Z);

		const float MyRadius = MyExtent[AxisIndex];

		const float AxisOverlap = (MyRadius + OtherRadius) - FMath::Abs(LocalCenter[AxisIndex]);

		Overlap[AxisIndex] = FMath::Clamp(AxisOverlap, 0.f, (2.f * FMath::Min(MyRadius, OtherRadius)));
	}

	const float OverlapVolume = Overlap.X * Overlap.Y * Overlap.Z;

	const float MyVolume = 8.f * MyExtent.X * MyExtent.Y * MyExtent.Z;
	const float OtherVolume = 8.f * InOtherExtent.X * InOtherExtent.Y * InOtherExtent.Z;

	const float MinVolume = FMath::Min(MyVolume, OtherVolume);

	return (MinVolume > KINDA_SMALL_NUMBER) ? (OverlapVolume / MinVolume) : 0.f;
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

			const FVector ExpectedNormal = IsDown ? FVector::UpVector : (FVector::UpVector * -1.f);

			const float NormalDot = FMath::Clamp(FVector::DotProduct(Hit.ImpactNormal, ExpectedNormal), -1.f, 1.f);
			const float SurfaceAngle = FMath::RadiansToDegrees(FMath::Acos(NormalDot));
			
			bool CanBuildFromProbe = (IsHit && SurfaceAngle <= BuildProbeData.MaxSurfaceAngle);
			
			if (const ADC1BuildActor* HitBuildActor = Cast<ADC1BuildActor>(Hit.GetActor()))
			{
				CanBuildFromProbe &= HitBuildActor->GetIsBuildableSurface();
			}


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
	TArray<UPrimitiveComponent*> OverlapComps;
	this->GetOverlappingComponents(OverlapComps);

	const ADC1BuildActor* MyBuild = GetOwner<ADC1BuildActor>();
	if (false == ::IsValid(MyBuild))
	{
		return false;
	}

	for (const UPrimitiveComponent* OverlapComp: OverlapComps)
	{
		const AActor* OtherActor = OverlapComp->GetOwner();
		if (OtherActor == GetOwner())
		{
			continue;
		}

		// 스냅 중이면 건축물끼리 살짝 겹치는 건 정상. 판정은 액터로, 측정은 체크박스로
		if (true == IsAutoSnap)
		{
			const ADC1BuildActor* OtherBuild = Cast<ADC1BuildActor>(OtherActor);
			
			if (false == ::IsValid(OtherBuild) || true == MyBuild->CheckAutoSnapOverlap(OtherBuild, AutoSnapOverlapAllowableRatio))
			{
				continue;
			}	
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


const FTransform UDC1BuildCheckComponent::MakeSnapSocketLocalTransform(const FDC1BuildSnapSocketData& InData) const
{
	const FVector CheckBoxExtent = GetUnscaledBoxExtent();
	const FVector LocalLocation = CheckBoxExtent * InData.SnapDirection;

	const FVector AttachAxis = InData.AttachAxis.GetSafeNormal();
	const FVector TangentAxis = InData.TangentAxis.GetSafeNormal();

	if (false == FMath::IsNearlyZero(FVector::DotProduct(AttachAxis, TangentAxis)))
	{
		UE_LOG(LogTemp, Warning, TEXT("SnapData Axis Not Orthogonal : %s"), *GetOwner()->GetName());
		return FTransform::Identity;
	}

	const FQuat LocalRotation = FRotationMatrix::MakeFromXZ(AttachAxis, TangentAxis).ToQuat();

	return FTransform(LocalRotation, LocalLocation);
}

void UDC1BuildCheckComponent::DrawDebugSockets()
{
	const UWorld* World = GetWorld();
	if (false == ::IsValid(World))
	{
		return;
	}

	DrawDebugBox(World, GetComponentLocation(), GetScaledBoxExtent(), GetComponentQuat(), FColor::Silver);

	TArray<FDC1BuildSnapSocket> Sockets;
	
	if (false == GetBuildSocketsWorld(Sockets))
	{
		return;
	}

	for (const FDC1BuildSnapSocket& Socket : Sockets)
	{
		const FVector Location = Socket.SocketTransform.GetLocation();
		const FRotator Rotator = Socket.SocketTransform.GetRotation().Rotator();

		DrawDebugCoordinateSystem(World, Location, Rotator, 40.f, false, -1.f, 0, 1.f);
		DrawDebugSphere(World, Location, 20.f, 8, FColor::White);
	}
}
