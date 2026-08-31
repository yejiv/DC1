// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Build/DC1BuildPlacementComponent.h"
#include "Actor/BuildActor/DC1BuildActor.h"

#include "Helper/DC1Helper.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Character/Player/DC1PlayerController.h"
#include "Tags/Input/DC1GameplayTag_Input.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "System/DC1AssetManager.h"
#include "Data/DA/Input/DC1InputData.h"

UDC1BuildPlacementComponent::UDC1BuildPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	SetIsReplicatedByDefault(true);
}

void UDC1BuildPlacementComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheBuildRows();
	SetUpBuildInput();
}

void UDC1BuildPlacementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelPlacement();

	Super::EndPlay(EndPlayReason);
}

void UDC1BuildPlacementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (false == ::IsValid(PreviewActor))
	{
		return;
	}

	FHitResult SurfaceHit;
	TraceSurface(SurfaceHit);

	PreviewActor->SetActorTransform(PreviewActor->ComputePlaceTransform(SurfaceHit, GetPlaceYaw()));

	CanBuilding = PreviewActor->UpdatePreview();
}

bool UDC1BuildPlacementComponent::StartPlacement(const FGameplayTag& InBuildTag)
{
	if (true == ::IsValid(PreviewActor))
	{
		CancelPlacement();
	}

	const FDC1BuildRow* BuildRow = FindBuildRow(InBuildTag);
	if (nullptr == BuildRow)
	{
		return false;
	}

	PreviewActor = SpawnBuildActor(*BuildRow, EDC1BuildState::Preview, FTransform::Identity);
	if (nullptr == PreviewActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildPlacementComponent :: Cant Spawn Build Actor : %s"), *InBuildTag.ToString());
		return false;
	}

	PreviewActor->SetReplicates(false);

	CurrentBuildTag = InBuildTag;
	CurrentYaw = 0.f;

	SetComponentTickEnabled(true);

	// Build IMC 추가, State_Build Tag 추가, InteractComp 비활성화
	SetBuildMode(true);

	return true;
}

void UDC1BuildPlacementComponent::CancelPlacement()
{
	SetComponentTickEnabled(false);

	if (true == ::IsValid(PreviewActor))
	{
		PreviewActor->Destroy();
	}

	PreviewActor = nullptr;
	CurrentBuildTag = FGameplayTag::EmptyTag;
	CurrentYaw = 0.f;
	CanBuilding = false;

	// Build IMC 제거, State_Build Tag 제거, InteractComp 활성화
	SetBuildMode(false);
}

void UDC1BuildPlacementComponent::ConfirmPlacement()
{
	if (false == ::IsValid(PreviewActor) || false == CanBuilding)
	{
		return;
	}

	Server_PlaceBuild(CurrentBuildTag, PreviewActor->GetActorTransform());
}

void UDC1BuildPlacementComponent::Server_PlaceBuild_Implementation(const FGameplayTag& InBuildTag, const FTransform& InTransform)
{
	const FDC1BuildRow* BuildRow = FindBuildRow(InBuildTag);
	if (nullptr == BuildRow)
	{
		return;
	}

	const APawn* Pawn = GetOwnerPawn();
	if (nullptr == Pawn)
	{
		return;
	}

	const FVector PawnLocation = Pawn->GetActorLocation();
	const FVector ActorLocation = InTransform.GetLocation();

	const float Distance = FVector::Dist2D(PawnLocation, ActorLocation);

	// 서버 검증 여유 수치
	if (Distance > ( PlaceDistance * DistanceMarginFactor ))
	{
		return;
	}

	// TODO Cost 감소
	// ..

	EDC1BuildState ConfirmState = (BuildRow->RequiredWork > 0.f) ? EDC1BuildState::Construction : EDC1BuildState::Completed;
	SpawnBuildActor(*BuildRow, ConfirmState, InTransform);
}

void UDC1BuildPlacementComponent::CacheBuildRows()
{
	TArray<FDC1BuildRow> AllRows;
	if (false == DC1DataTableHelper::GetAllRows<FDC1BuildRow>(DC1DataTableTags::DataTable_UI_Build, AllRows))
	{
		UE_LOG(LogTemp, Warning, TEXT("Build DataTable is NULL"));
		return;
	}

	BuildRowsByTag.Empty();

	for (const FDC1BuildRow& Row : AllRows)
	{
		const FGameplayTag& BuildTag = Row.BuildTag;
		if (false == BuildTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Build Tag is Invalid"));
			continue;
		}

		if (nullptr != BuildRowsByTag.Find(BuildTag))
		{
			UE_LOG(LogTemp, Warning, TEXT("Duplication BuildTag"));
			continue;
		}

		BuildRowsByTag.Add(BuildTag, Row);
	}
}

const FDC1BuildRow* UDC1BuildPlacementComponent::FindBuildRow(const FGameplayTag& InBuildTag) const
{
	const FDC1BuildRow* BuildRow = BuildRowsByTag.Find(InBuildTag);
	if (nullptr == BuildRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildPlacementComponent :: Cant Find BuildRow : %s"), *InBuildTag.ToString());
		return nullptr;
	}

	return BuildRow;
}

FDC1BuildActorData UDC1BuildPlacementComponent::MakeBuildActorData(const FDC1BuildRow& InRow) const
{
	FDC1BuildActorData Data;
	Data.BuildTag = InRow.BuildTag;
	Data.RequiredWork = InRow.RequiredWork;
	Data.BuildCosts = InRow.BuildCosts;

	return Data;
}

ADC1BuildActor* UDC1BuildPlacementComponent::SpawnBuildActor(const FDC1BuildRow& InRow, EDC1BuildState InState, const FTransform& InTransform)
{
	// SoftclassPtr 동기 Load
	if (nullptr == InRow.BuildActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Empty Build Actor Class : %s"), *(InRow.BuildTag).ToString());
		return nullptr;
	}

	UClass* LoadedClass = InRow.BuildActorClass.LoadSynchronous();
	if (false == ::IsValid(LoadedClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed Load Build Actor  Class : %s"), *(InRow.BuildTag).ToString());
		return nullptr;
	}

	// State 세팅을 위해 SpawnActorDeferred | AlwaysSpawn - 스폰 위치에 다른 충돌체가 있어도 스폰
	ADC1BuildActor* NewBuildActor = GetWorld()->SpawnActorDeferred<ADC1BuildActor>(LoadedClass, InTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (nullptr == NewBuildActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cant Spawn Build Actor : %s"), *(InRow.BuildTag).ToString());
		return nullptr;
	}

	NewBuildActor->InitBuild(MakeBuildActorData(InRow), InState);
	NewBuildActor->FinishSpawning(InTransform);

	return NewBuildActor;
}

bool UDC1BuildPlacementComponent::TraceSurface(FHitResult& OutHit) const
{
	const APawn* Pawn = GetOwnerPawn();
	if (nullptr == Pawn)
	{
		return false;
	}

	// 컨트롤러가 바라보는 방향
	FVector FowardDirection = FRotationMatrix(FRotator(0.f, GetControlYaw(), 0.f)).GetUnitAxis(EAxis::X);
	
	// Pawn(Player) 위치
	FVector PawnLocation = Pawn->GetActorLocation();
	const float PawnFootZ = PawnLocation.Z - Pawn->GetSimpleCollisionHalfHeight();

	// Build Actor가 위치할 지점 ( X, Y 만 사용 )
	FVector TargetLocation = PawnLocation + (FowardDirection * PlaceDistance);

	// Build Actor Trace 지점들
	FVector TopPoint = FVector(TargetLocation.X, TargetLocation.Y, PawnFootZ + RangeUpHeight);
	FVector BottomPoint = FVector(TargetLocation.X, TargetLocation.Y, PawnFootZ - RangeDownHeight);

	bool IsDown = (PreviewActor->GetProbeDirection() == EDC1BuildProbeDirection::Down);
	
	// Direction에 따라 위 -> 아래 or 아래 -> 위
	FVector Start = IsDown ? TopPoint : BottomPoint;
	FVector End = IsDown ? BottomPoint : TopPoint;

	// Trace
	FHitResult Hit;

	FCollisionQueryParams HitParams;
	HitParams.AddIgnoredActor(Pawn);
	HitParams.AddIgnoredActor(PreviewActor);

	// 충돌 채널 추가 필요
	const bool IsHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_BuildSurface), HitParams);

	if (false == IsHit)
	{
		OutHit.ImpactPoint = FVector(TargetLocation.X, TargetLocation.Y, PawnFootZ);
		OutHit.ImpactNormal = IsDown ? FVector::UpVector : (FVector::UpVector * -1.f);
		
		return false;
	}

	OutHit = Hit;

	return true;
}

void UDC1BuildPlacementComponent::SetUpBuildInput()
{
	APlayerController* PC = GetOwner<APlayerController>();
	if (false == ::IsValid(PC) || false == PC->IsLocalController())
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (false == ::IsValid(EnhancedInputComponent))
	{
		return;
	}

	const UDC1InputData* InputData = UDC1AssetManager::GetAssetByTag<UDC1InputData>(DC1AssetTags::Asset_InputData);
	if (false == ::IsValid(InputData))
	{
		return;
	}

	if (const UInputAction* ConfirmAction = InputData->FindInputActionbyTag(DC1InputBuildTags::Input_Build_Confirm))
	{
		EnhancedInputComponent->BindAction(ConfirmAction, ETriggerEvent::Started, this, &ThisClass::ConfirmPlacement);
	}

	if (const UInputAction* CancelAction = InputData->FindInputActionbyTag(DC1InputBuildTags::Input_Build_Cancel))
	{
		EnhancedInputComponent->BindAction(CancelAction, ETriggerEvent::Started, this, &ThisClass::CancelPlacement);
	}

	if (const UInputAction* RotateAction = InputData->FindInputActionbyTag(DC1InputBuildTags::Input_Build_Rotate))
	{
		EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Triggered, this, &ThisClass::OnRotateInput);
	}
}

void UDC1BuildPlacementComponent::OnRotateInput(const FInputActionValue& Value)
{
	if (false == ::IsValid(PreviewActor))
	{
		return;
	}

	float InYaw = Value.Get<float>();

	CurrentYaw = FMath::UnwindDegrees(CurrentYaw + (InYaw * RotationStep));
}

void UDC1BuildPlacementComponent::SetBuildMode(bool InEnabled)
{
	ADC1PlayerController* PC = GetOwner<ADC1PlayerController>();
	if (nullptr != PC)
	{
		PC->SetIMCEnabled(DC1IMCTags::IMC_Build, InEnabled);
		PC->SetBuildMode(InEnabled);
	}
}

const APawn* UDC1BuildPlacementComponent::GetOwnerPawn() const
{
	const APlayerController* PC = GetOwner<APlayerController>();
	if (false == ::IsValid(PC))
	{
		return nullptr;
	}

	const APawn* Pawn = PC->GetPawn();
	if (false == ::IsValid(Pawn))
	{
		return nullptr;
	}

	return Pawn;
}

float UDC1BuildPlacementComponent::GetPlaceYaw() const
{
	float BaseYaw = IsSnapping ? SnapBaseYaw : GetControlYaw();

	return FMath::UnwindDegrees(BaseYaw + CurrentYaw);
}

float UDC1BuildPlacementComponent::GetControlYaw() const
{
	const APawn* Pawn = GetOwnerPawn();
	if (false == ::IsValid(Pawn))
	{
		return 0.f;
	}

	return Pawn->GetControlRotation().Yaw;;
}
