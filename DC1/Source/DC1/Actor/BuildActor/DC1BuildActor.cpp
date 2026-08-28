// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/DC1BuildActor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Component/Build/DC1BuildCheckComponent.h"

#include "Character/DC1Character.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "Tags/State/DC1GameplayTag_State.h"

#include "Subsystem/DC1UIManagerSubsystem.h"

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

	InteractCollider = CreateDefaultSubobject<UBoxComponent>("InteractCollider");
	InteractCollider->SetupAttachment(BuildMesh);
	InteractCollider->SetGenerateOverlapEvents(true);

	BuildCheckComp = CreateDefaultSubobject<UDC1BuildCheckComponent>("BuildCheckComp");
	BuildCheckComp->SetupAttachment(BuildMesh);

	SetRootComponent(BuildMesh);
}

EDC1BuildProbeDirection ADC1BuildActor::GetProbeDirection() const
{
	return (::IsValid(BuildCheckComp) ? BuildCheckComp->GetProbeDirection() : EDC1BuildProbeDirection::Down);
}

FTransform ADC1BuildActor::ComputePlaceTransform(const FHitResult& InHit, float InYaw) const
{
	return (::IsValid(BuildCheckComp) ? BuildCheckComp->ComputePlaceTransform(InHit, InYaw) : FTransform::Identity);
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

	// Completed 처리
	if (FlashAmount > 0.f)
	{
		FlashAmount -= (DeltaTime / CompleteFlashDuration);

		if (FlashAmount <= 0.f)
		{
			FlashAmount = 0.f;
			BuildCompleted();
		}
		else
		{
			SetMaterialFlash();
		}

		return;
	}

	// Construction 처리
	if (true == Workers.IsEmpty() || false == HasAuthority())
	{
		return;
	}

	const float TotalWorkSpeed = ComputeTotalWorkSpeed();

	if (TotalWorkSpeed > 0.f && BuildActorData.RequiredWork > 0.f && CurrentWork < BuildActorData.RequiredWork)
	{
		CurrentWork += (TotalWorkSpeed * DeltaTime);

		BuildProgress = (CurrentWork / BuildActorData.RequiredWork);

		if (BuildProgress >= 1.f)
		{
			BuildProgress = 1.f;

			SetState(EDC1BuildState::Completed);
		}

		SetMaterialBuildProgress();
 	}
}

UPrimitiveComponent* ADC1BuildActor::GetInteractCollider() const
{
	if (false == ::IsValid(InteractCollider))
	{
		return nullptr;
	}

	return InteractCollider;
}

FGameplayTag ADC1BuildActor::GetInteractAbilityTag() const
{
	if (BuildState == EDC1BuildState::Construction)
	{
		return InteractAbilityTag;
	}

	return FGameplayTag::EmptyTag;
}

void ADC1BuildActor::OnInteractableFocused()
{
	if (false == OnFocusedUITag.IsValid())
	{
		return;
	}

	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
		return;
	}

	// TODO Push World UI
	//UIManager->PushScreen(OnFocusedUITag);
}

void ADC1BuildActor::OnInteractableUnfocused()
{
	if (false == OnFocusedUITag.IsValid())
	{
		return;
	}

	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
		return;
	}

	// TODO Pop World UI
}

void ADC1BuildActor::InitBuild(const FDC1BuildActorData& InBuildActorData, EDC1BuildState InState)
{
	BuildActorData = InBuildActorData;
	BuildState = InState;
}

void ADC1BuildActor::AddWorker(const ADC1Character* InWorker)
{
	if (false == ::IsValid(InWorker) || false == HasAuthority() || BuildState != EDC1BuildState::Construction)
	{
		return;
	}

	Workers.Add(InWorker);
}

void ADC1BuildActor::RemoveWorker(const ADC1Character* InWorker)
{
	if (nullptr == InWorker || false == HasAuthority())
	{
		return;
	}

	Workers.Remove(InWorker);
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

void ADC1BuildActor::BuildDestroy()
{
	// TODO Despawn, Cost Return
	CancelWorkersAbility();
	
	Workers.Empty();
}

void ADC1BuildActor::OnRep_BuildState()
{
	if (true == HasActorBegunPlay())
	{
		ApplyState();
	}
}

void ADC1BuildActor::OnRep_BuildProgress()
{
	SetMaterialBuildProgress();
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

	switch (BuildState)
	{
	case EDC1BuildState::Preview:
	{
		BuildMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		InteractCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CreateTranslucentMIDs();
		SetMaterialTranslucentColor(CanBuilding ? PreviewValidColor : PreviewInvalidColor);
		SetActorTickEnabled(false);
		break;
	}
	case EDC1BuildState::Construction:
	{
		BuildMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		InteractCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		CreateTranslucentMIDs();
		SetMaterialTranslucentColor(ConstructionColor);
		SetMaterialBuildProgress();
		SetActorTickEnabled(true);
		break;
	}
	case EDC1BuildState::Completed:
	{
		BuildMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		InteractCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		if (true == BuildMIDs.IsEmpty())
		{
			SetActorTickEnabled(false);
		}
		else
		{
			FlashAmount = 1.f;
			SetActorTickEnabled(true);
		}

		OnBuildCompleted.Broadcast();

		Workers.Empty();

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

void ADC1BuildActor::BuildCompleted()
{
	if (true == ::IsValid(BuildMesh))
	{
		BuildMesh->EmptyOverrideMaterials();
	}

	BuildMIDs.Reset();

	SetActorTickEnabled(false);
}

void ADC1BuildActor::CancelWorkersAbility()
{
	FGameplayTagContainer WorkTags;
	WorkTags.AddTag(DC1StateTags::State_Build_Working);

	for (TWeakObjectPtr<const ADC1Character> Worker : Workers)
	{
		const ADC1Character* WorkCharacter = Worker.Get();
		if (false == ::IsValid(WorkCharacter))
		{
			continue;
		}

		UAbilitySystemComponent* ASC = WorkCharacter->GetAbilitySystemComponent();
		if (false == ::IsValid(ASC))
		{
			continue;
		}
		
		ASC->CancelAbilities(&WorkTags);
	}
}

float ADC1BuildActor::ComputeTotalWorkSpeed()
{
	float TotalWorkSpeed = 0.f;

	for (TSet<TWeakObjectPtr<const ADC1Character>>::TIterator Iter = Workers.CreateIterator(); Iter; ++Iter)
	{
		const ADC1Character* Worker = Iter->Get();
		if (false == ::IsValid(Worker))
		{
			Iter.RemoveCurrent();
			continue;
		}

		const UAbilitySystemComponent* ASC = Worker->GetAbilitySystemComponent();
		if (false == ::IsValid(ASC) || ASC->HasMatchingGameplayTag(DC1StateTags::State_Dead))
		{
			continue;
		}

		TotalWorkSpeed += ASC->GetNumericAttribute(UDC1AttributeSet::GetWorkSpeedAttribute());
	}

	return TotalWorkSpeed;
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

void ADC1BuildActor::SetMaterialBuildProgress()
{
	int32 NumMIDs = BuildMIDs.Num();

	for (int32 i = 0; i < NumMIDs; ++i)
	{
		if (false == ::IsValid(BuildMIDs[i]))
		{
			continue;
		}

		if (false == MaterialProgressRanges.IsValidIndex(i))
		{
			UE_LOG(LogTemp, Warning, TEXT("Material None Progress Range : %d"), i);
			continue;
		}

		const FVector2D& Range = MaterialProgressRanges[i];
		const float RangeSize = (Range.Y - Range.X);

		float MaterialProgress = 0.f;

		if (RangeSize > KINDA_SMALL_NUMBER)
		{
			MaterialProgress = FMath::Clamp(((BuildProgress - Range.X) / RangeSize), 0.f, 1.f);
		}

		BuildMIDs[i]->SetScalarParameterValue(DC1BuildMaterialParams::BuildProgress, MaterialProgress);
	}
}

void ADC1BuildActor::SetMaterialFlash()
{
	for (UMaterialInstanceDynamic* MID : BuildMIDs)
	{
		if (true == ::IsValid(MID))
		{
			MID->SetScalarParameterValue(DC1BuildMaterialParams::FlashAmount, FlashAmount);
		}
	}
}
