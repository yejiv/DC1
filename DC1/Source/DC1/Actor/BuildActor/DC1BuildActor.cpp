// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/DC1BuildActor.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Component/Build/DC1BuildCheckComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values
ADC1BuildActor::ADC1BuildActor()
{
 	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;

	SetReplicateMovement(false);
	
	BuildMesh = CreateDefaultSubobject<UStaticMeshComponent>("BuildMesh");
	BuildMesh->SetMobility(EComponentMobility::Movable);

	BuildCheckComp = CreateDefaultSubobject<UDC1BuildCheckComponent>("BuildCheckComp");
	BuildCheckComp->SetupAttachment(BuildMesh);

	SetRootComponent(BuildMesh);
}

const EDC1BuildProbeDirection ADC1BuildActor::GetProbeDirection() const
{
	return ::IsValid(BuildCheckComp) ? BuildCheckComp->GetProbeDirection() : EDC1BuildProbeDirection::Down;
}

const FVector ADC1BuildActor::GetCheckBoxCenter() const
{
	return ::IsValid(BuildCheckComp) ? BuildCheckComp->GetCheckBoxCenter() : FVector::ZeroVector;
}

const FTransform ADC1BuildActor::ComputePlaceTransform(const FHitResult& InHit, float InYaw) const
{
	return ::IsValid(BuildCheckComp) ? BuildCheckComp->ComputePlaceTransform(InHit, InYaw) : FTransform::Identity;
}

const float ADC1BuildActor::GetExtentCenterToDirection(const FVector& InWorldDirection, const FQuat& InQuat) const
{
	return ::IsValid(BuildCheckComp) ? BuildCheckComp->GetExtentCenterToDirection(InWorldDirection, InQuat) : 0.f;
}

// Called when the game starts or when spawned
void ADC1BuildActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (true == HasAuthority() && false == BuildActorData.BuildTag.IsValid())
	{
		BuildState = EDC1BuildState::Completed;
	}

	ApplyState();
}

void ADC1BuildActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, BuildActorData, COND_InitialOnly);

	DOREPLIFETIME(ThisClass, BuildState);
}

// Called every frame

void ADC1BuildActor::InitBuild(const FDC1BuildActorData& InBuildActorData)
{
	BuildActorData = InBuildActorData;

	BuildState = EDC1BuildState::Completed;
}

void ADC1BuildActor::InitPreview(const FDC1BuildActorData& InBuildActorData)
{
	BuildActorData = InBuildActorData;

	BuildState = EDC1BuildState::Preview;
}


bool ADC1BuildActor::UpdatePreview(bool InIsInRange)
{
	const bool CurrentCanBuilding = BuildCheckComp->CheckCanBuild() && (true == InIsInRange);

	if (CanBuilding != CurrentCanBuilding)
	{
		CanBuilding = CurrentCanBuilding;

		FLinearColor TranslucentColor = CanBuilding ? PreviewValidColor : PreviewInvalidColor;
		SetMaterialTranslucentColor(TranslucentColor);
	}

	return CanBuilding;
}

bool ADC1BuildActor::SelectSnapPointByRay(const FVector& InRayStart, const FVector& InRayEnd, const FCollisionQueryParams& InParams, FDC1BuildSnapSocket& OutSnapSocket) const
{
	if (false == ::IsValid(BuildCheckComp))
	{
		return false;
	}

	FHitResult Hit;

	if (false == BuildCheckComp->LineTraceComponent(Hit, InRayStart, InRayEnd, InParams))
	{
		return false;
	}

	/*if (true == Hit.bStartPenetrating)
	{
		return false;
	}*/

	OutSnapSocket = BuildCheckComp->MakeSnapSocketSide(Hit.ImpactNormal);

	return true;
}

bool ADC1BuildActor::SelectSnapPointByPoint(const FVector& InWorldLocation, FDC1BuildSnapSocket& OutSnapSocket) const
{
	if (false == ::IsValid(BuildCheckComp))
	{
		return false;
	}

	OutSnapSocket = BuildCheckComp->MakeNearestSnapSocketSide(InWorldLocation);

	return true;
}

void ADC1BuildActor::BuildDestroy()
{
	// TODO Despawn, Cost Return
}

void ADC1BuildActor::OnRep_BuildState()
{
	if (true == HasActorBegunPlay())
	{
		ApplyState();
	}
}


void ADC1BuildActor::SetState(EDC1BuildState InNewState)
{
	if (BuildState == InNewState)
	{
		return;
	}

	BuildState = InNewState;

	if (true == HasActorBegunPlay())
	{
		ApplyState();
	}
}

void ADC1BuildActor::ApplyState()
{
	if (BuildState == EDC1BuildState::Default || false == ::IsValid(BuildMesh))
	{
		return;
	}

	switch (BuildState)
	{
	case EDC1BuildState::Preview:
	{
		BuildMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CreateTranslucentMIDs();
		SetMaterialTranslucentColor(CanBuilding ? PreviewValidColor : PreviewInvalidColor);
		SetActorTickEnabled(false);
		break;
	}
	case EDC1BuildState::Completed:
	{
		BuildMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SetActorTickEnabled(false);

		break;
	}
	}
}

void ADC1BuildActor::CreateTranslucentMIDs()
{
	if (false == BuildMIDs.IsEmpty())
	{
		return;
	}

	// Create DMI
	if (false == ::IsValid(BuildMesh) || false == ::IsValid(TranslucentMaterial))
	{
		return;
	}

	const int32 NumMaterials = BuildMesh->GetNumMaterials();

	for (int32 i = 0; i < NumMaterials; ++i)
	{
		UMaterialInterface* OriginalMaterial = BuildMesh->GetMaterial(i);

		UMaterialInstanceDynamic* MID = BuildMesh->CreateDynamicMaterialInstance(i, TranslucentMaterial);

		if (false == ::IsValid(MID))
		{
			continue;
		}

		UTexture* SourceTexture = nullptr;
		float UseTexture = 0.f;

		if (true == ::IsValid(OriginalMaterial))
		{
			OriginalMaterial->GetTextureParameterValue(DC1BuildMaterialParams::SourceBaseColorParamName, SourceTexture);
		}

		if (true == ::IsValid(SourceTexture))
		{
			MID->SetTextureParameterValue(DC1BuildMaterialParams::BaseColorTexture, SourceTexture);
			UseTexture = 1.f;
		}

		MID->SetScalarParameterValue(DC1BuildMaterialParams::UseTexture, UseTexture);

		BuildMIDs.Add(MID);
	}
}


void ADC1BuildActor::SetMaterialTranslucentColor(const FLinearColor& InColor)
{
	for (UMaterialInstanceDynamic* MID : BuildMIDs)
	{
		if (true == ::IsValid(MID))
		{
			MID->SetVectorParameterValue(DC1BuildMaterialParams::TranslucentColor, InColor);
		}
	}
}


