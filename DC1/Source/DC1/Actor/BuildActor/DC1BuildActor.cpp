// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/DC1BuildActor.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Component/Build/DC1BuildCheckComponent.h"
#include "Component/Build/DC1BuildSnapComponent.h"
#include "Component/Build/DC1BuildLinkComponent.h"

#include "Define/DC1BuildSupportHelper.h"

#include "Net/UnrealNetwork.h"

#include "Actor/BuildActor/Destroy/DC1BuildDestroyActor.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

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

	BuildSnapComp = CreateDefaultSubobject<UDC1BuildSnapComponent>("BuildSnapComp");
	BuildSnapComp->SetupAttachment(BuildCheckComp);

	BuildLinkComp = CreateDefaultSubobject<UDC1BuildLinkComponent>("BuildLinkComp");

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

const bool ADC1BuildActor::GetIsAutoSnap() const
{
	return ::IsValid(BuildCheckComp) ? BuildCheckComp->GetIsAutoSnap() : false;
}

const float ADC1BuildActor::GetSnappedRotationStep() const
{
	return ::IsValid(BuildCheckComp) ? BuildCheckComp->GetSnappedRotationStep() : 0.f;
}

const bool ADC1BuildActor::GetBuildSocketsToWorld(TArray<FDC1BuildSnapSocket>& OutSockets) const
{
	return ::IsValid(BuildSnapComp) ? BuildSnapComp->GetBuildSocketsToWorld(OutSockets) : false;
}

void ADC1BuildActor::SetCheckSurface(bool InIsCheckSurface)
{
	if (true == ::IsValid(BuildCheckComp))
	{
		BuildCheckComp->SetCheckSurface(InIsCheckSurface || HasPillar);
	}
}

const bool ADC1BuildActor::GetCheckBoxWorld(FDC1BuildBox& OutBox) const
{
	if (false == ::IsValid(BuildCheckComp))
	{
		return false;
	}

	OutBox.Transform = BuildCheckComp->GetComponentTransform();
	OutBox.Extent = BuildCheckComp->GetScaledBoxExtent();

	return true;
}

const bool ADC1BuildActor::IsBlockedAt(const FTransform& InActorTransform, const TArray<FDC1BuildBox>& InBlockers) const
{
	if (false == ::IsValid(BuildCheckComp))
	{
		return false;
	}

	const FTransform BoxTransform = BuildCheckComp->GetRelativeTransform() * InActorTransform;

	return BuildCheckComp->IsBlockedAt(BoxTransform, InBlockers);
}

const bool ADC1BuildActor::GatherSurfaceSupporters(TArray<ADC1BuildActor*>& OutSupporters, bool& OutIsOnGround) const
{
	if (false == ::IsValid(BuildCheckComp))
	{
		return false;
	}

	BuildCheckComp->GatherSurfaceSupporters(OutSupporters, OutIsOnGround);

	return true;
}

// Called when the game starts or when spawned
void ADC1BuildActor::BeginPlay()
{
	Super::BeginPlay();
	
	for (UActorComponent* Comp : GetComponents())
	{
		UDC1BuildCheckComponent* CheckComp = Cast<UDC1BuildCheckComponent>(Comp);
		if (true == ::IsValid(CheckComp) && CheckComp != BuildCheckComp)
		{
			SubBuildCheckComps.Add(CheckComp);

			HasPillar |= CheckComp->GetIsPillar();
		}
	}

	if (true == HasAuthority() && false == BuildActorData.BuildTag.IsValid())
	{
		BuildState = EDC1BuildState::Completed;
	}

	ApplyState();
}

void ADC1BuildActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(DestroySequenceTimer);

	if (true == IsDestroying)
	{
		FinishDestroySequence();
	}

	Super::EndPlay(EndPlayReason);
}

void ADC1BuildActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, BuildActorData, COND_InitialOnly);

	DOREPLIFETIME(ThisClass, BuildState);
}

void ADC1BuildActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (true == ::IsValid(BuildSnapComp))
	{
		BuildSnapComp->LayoutSockets();
	}
}


void ADC1BuildActor::InitBuild(const FDC1BuildActorData& InBuildActorData)
{
	BuildActorData = InBuildActorData;

	BuildState = EDC1BuildState::Completed;
}

void ADC1BuildActor::BuildDestroy()
{
	Destroy();
}

void ADC1BuildActor::FinishDestroySequence()
{
	IsDestroying = false;

	OnDestroyFinished.Broadcast(this);
}

void ADC1BuildActor::InitPreview(const FDC1BuildActorData& InBuildActorData)
{
	BuildActorData = InBuildActorData;

	BuildState = EDC1BuildState::Preview;
}

void ADC1BuildActor::BeginDestroySequence()
{
	if (true == IsDestroying)
	{
		return;
	}

	// TODO Cost Return

	IsDestroying = true;

	SetState(EDC1BuildState::Destroying);

	if (DestroySequenceDuration > 0.f)
	{
		GetWorldTimerManager().SetTimer(DestroySequenceTimer, this, &ThisClass::FinishDestroySequence, DestroySequenceDuration);
	}
	else
	{
		DestroySequenceTimer = GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::FinishDestroySequence);
	}
}


void ADC1BuildActor::SetDismantleHighlight(bool InIsHighlighted)
{
	if (false == ::IsValid(BuildMesh))
	{
		return;
	}

	UMaterialInterface* OverlayMaterial = InIsHighlighted ? DismantleOverlayMaterial : nullptr;
	BuildMesh->SetOverlayMaterial(OverlayMaterial);
}

bool ADC1BuildActor::UpdatePreview(bool InIsInRange, const APlayerState* InBuilder)
{
	bool IsSubCheck = true;

	for (UDC1BuildCheckComponent* SubCheck : SubBuildCheckComps)
	{
		IsSubCheck &= (SubCheck->CheckOverlap());
	}

	const bool CurrentCanBuilding = IsSubCheck && BuildCheckComp->CheckCanBuild() && (true == InIsInRange) && DC1BuildSupportHelper::HasSupport(this) && CanPlaceAt(InBuilder);


	if (CanBuilding != CurrentCanBuilding)
	{
		CanBuilding = CurrentCanBuilding;

		FLinearColor TranslucentColor = CanBuilding ? PreviewValidColor : PreviewInvalidColor;
		SetMaterialTranslucentColor(TranslucentColor);
	}

	return CanBuilding;
}

bool ADC1BuildActor::TraceCheckBox(const FVector& InRayStart, const FVector& InRayEnd, const FCollisionQueryParams& InParams, float InInflate, FVector& OutImpactPoint) const
{
	if (false == ::IsValid(BuildCheckComp))
	{
		return false;
	}

	const FTransform BoxTransform = BuildCheckComp->GetComponentTransform();
	const FVector BoxExtent = BuildCheckComp->GetScaledBoxExtent();

	const FBox	LocalBox(BoxExtent * -1.f, BoxExtent);
	const FVector LocalRayStart = BoxTransform.InverseTransformPositionNoScale(InRayStart);
	const FVector LocalRayEnd = BoxTransform.InverseTransformPositionNoScale(InRayEnd);

	FVector LocalHit = FVector::ZeroVector;
	FVector LocalNormal = FVector::ZeroVector;

	float HitTime = 0.f;

	const bool IsEnter = FMath::LineExtentBoxIntersection(LocalBox, LocalRayStart, LocalRayEnd, FVector(InInflate), LocalHit, LocalNormal, HitTime);
	
	if(true == IsEnter && HitTime > 0.f)
	{
		OutImpactPoint = BoxTransform.TransformPositionNoScale(LocalHit);

		return true;
	}

	FHitResult Hit;

	if (false == BuildCheckComp->LineTraceComponent(Hit, InRayStart, InRayEnd, InParams))
	{
		return false;
	}

	OutImpactPoint = Hit.ImpactPoint;

	return true;
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
	if (BuildState == InNewState || BuildState == EDC1BuildState::Destroying)
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
	case EDC1BuildState::Destroying:
	{
		SetActorTickEnabled(false);
		PlayDestroyFX();
		break;
	}
	}
}

void ADC1BuildActor::PlayDestroyFX()
{
	if (NM_DedicatedServer == GetNetMode() || false == ::IsValid(BuildMesh))
	{
		return;
	}

	BuildMesh->SetHiddenInGame(true);

	if (true == ::IsValid(DestroyClass) && true == ::IsValid(DestroyGeometryCollection))
	{
		const FTransform SpawnTransform = BuildMesh->GetComponentTransform();
		ADC1BuildDestroyActor* DestroyActor = GetWorld()->SpawnActorDeferred<ADC1BuildDestroyActor>(DestroyClass, SpawnTransform);
		if (true == ::IsValid(DestroyActor))
		{
			DestroyActor->InitDestroy(DestroyGeometryCollection);
			DestroyActor->FinishSpawning(SpawnTransform);
		}
	}

	if (true == ::IsValid(DestroyDustFX))
	{
		UNiagaraComponent* Dust = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, DestroyDustFX, BuildMesh->Bounds.Origin, GetActorRotation());
		if (true == ::IsValid(Dust))
		{
			// Dust Setting 
			// Dust->SetVariableVec3("Name", BuildMesh->Bounds.BoxExtent);
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
		MID->SetScalarParameterValue(DC1BuildMaterialParams::BuildProgress, 0.f);
		MID->SetScalarParameterValue(DC1BuildMaterialParams::FlashAmount, 0.f);

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


