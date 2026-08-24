// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/DC1BuildActor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Component/Build/DC1BuildCheckComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values
ADC1BuildActor::ADC1BuildActor()
{
 	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;

	SetReplicateMovement(false);
	
	BuildMesh = CreateDefaultSubobject<UStaticMeshComponent>("BuildMesh");
	BuildMesh->SetMobility(EComponentMobility::Movable);

	InteractCollider = CreateDefaultSubobject<USphereComponent>("InteractCollider");
	InteractCollider->SetupAttachment(BuildMesh);

	BuildCheckComp = CreateDefaultSubobject<UDC1BuildCheckComponent>("BuildCheckComp");
	BuildCheckComp->SetupAttachment(BuildMesh);

	SetRootComponent(BuildMesh);
}

// Called when the game starts or when spawned
void ADC1BuildActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (true == HasAuthority() && false == BuildActorData.BuildTag.IsValid())
	{
		BuildState = EDC1BuildState::Completed;
		BuildProgress = 1.f;
	}

	// Create DMI
	if (true == ::IsValid(BuildMesh))
	{
		int32 NumMaterials = BuildMesh->GetNumMaterials();

		for(int32 i = 0; i < NumMaterials; ++i)
		{
			UMaterialInstanceDynamic* MID = BuildMesh->CreateAndSetMaterialInstanceDynamic(i);

			if (true == ::IsValid(MID))
			{
				BuildMIDs.Add(MID);
			}
		}
	}

	CurrentWork = 0.f;

	ApplyState();
}

void ADC1BuildActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, BuildActorData, COND_InitialOnly);

	DOREPLIFETIME(ThisClass, BuildState);
	DOREPLIFETIME(ThisClass, BuildProgress);
}

// Called every frame
void ADC1BuildActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	BuildCheckComp->CheckCanBuild();

	// Construction 처리
	if (WorkSpeed <= 0.f || false == HasAuthority())
	{
		return;
	}

	if (BuildActorData.RequiredWork > 0.f && CurrentWork < BuildActorData.RequiredWork)
	{
		CurrentWork += (WorkSpeed * DeltaTime);

		BuildProgress = (CurrentWork / BuildActorData.RequiredWork);

		if (BuildProgress >= 1.f)
		{
			BuildProgress = 1.f;

			SetState(EDC1BuildState::Completed);
		}

		SetMaterialBuildProgress();
 	}
}

void ADC1BuildActor::InitBuild(const FDC1BuildActorData& InBuildActorData, EDC1BuildState InState)
{
	BuildActorData = InBuildActorData;
	BuildState = InState;
}

bool ADC1BuildActor::UpdatePreview()
{
	const bool CurrentCanBuilding = BuildCheckComp->CheckCanBuild();

	if (CanBuilding != CurrentCanBuilding)
	{
		CanBuilding = CurrentCanBuilding;

		FLinearColor TranslucentColor = CanBuilding ? PreviewValidColor : PreviewInvalidColor;
		SetMaterialTranslucentColor(TranslucentColor);
	}

	return CanBuilding;
}

void ADC1BuildActor::AddBuildProgress(float InWork)
{
}

void ADC1BuildActor::BuildDestroy()
{
}

void ADC1BuildActor::OnRep_BuildState()
{
}

void ADC1BuildActor::OnRep_BuildProgress()
{
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
	if (BuildState == EDC1BuildState::Default || false == ::IsValid(BuildMesh) || false == ::IsValid(InteractCollider))
	{
		return;
	}

	bool	IsPreview = (BuildState == EDC1BuildState::Preview);
	
	BuildMesh->SetCollisionEnabled(IsPreview ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
	InteractCollider->SetCollisionEnabled(IsPreview ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryOnly);
	BuildCheckComp->SetCollisionEnabled(IsPreview ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);

	bool	IsTickEnabled = (BuildState == EDC1BuildState::Construction) ? true : false;
	SetActorTickEnabled(true);
}

void ADC1BuildActor::SetMaterialTranslucentColor(const FLinearColor& InColor)
{
	for (UMaterialInstanceDynamic* MID : BuildMIDs)
	{
		if (true == ::IsValid(MID))
		{
			MID->SetVectorParameterValue(DC1BuildMaterialParams::TranslucentColor, InColor);
			MID->SetScalarParameterValue(DC1BuildMaterialParams::BuildProgress, BuildProgress);
		}
	}
}

void ADC1BuildActor::SetMaterialBuildProgress()
{
	for (UMaterialInstanceDynamic* MID : BuildMIDs)
	{
		if (true == ::IsValid(MID))
		{
			MID->SetScalarParameterValue(DC1BuildMaterialParams::BuildProgress, BuildProgress);
		}
	}
}