// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Build/DC1BuildPlacementComponent.h"
#include "Actor/BuildActor/DC1BuildActor.h"

#include "Helper/DC1Helper.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

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
}

void UDC1BuildPlacementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopPlacement();

	Super::EndPlay(EndPlayReason);
}

void UDC1BuildPlacementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (false == IsPlacing())
	{
		return;
	}

	FHitResult SurfaceHit;
	TraceSurface(SurfaceHit);

	PreviewActor->SetActorTransform(PreviewActor->ComputePlaceTransform(SurfaceHit, CurrentYaw));

	CanBuilding = PreviewActor->UpdatePreview();
}

bool UDC1BuildPlacementComponent::IsPlacing() const
{
	return ::IsValid(PreviewActor);
}

bool UDC1BuildPlacementComponent::StartPlacement(const FGameplayTag& InBuildTag)
{
	if (true == IsPlacing())
	{
		StopPlacement();
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

	// IMC_Build Add - PC에 구현

	return true;
}

void UDC1BuildPlacementComponent::StopPlacement()
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
}

void UDC1BuildPlacementComponent::ConfirmPlacement()
{
}

void UDC1BuildPlacementComponent::AddRotation(float InDeltaYaw)
{
}

void UDC1BuildPlacementComponent::Server_PlaceBuild_Implementation(const FGameplayTag& InBuildTag, const FTransform& InTransform)
{
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
	APlayerController* PC = GetOwner<APlayerController>();
	if (false == ::IsValid(PC))
	{
		return false;
	}

	APawn* Pawn = PC->GetPawn();
	if (false == ::IsValid(Pawn))
	{
		return false;
	}

	// 컨트롤러가 바라보는 방향
	FVector FowardDirection = FRotationMatrix(FRotator(0.f, PC->GetControlRotation().Yaw, 0.f)).GetUnitAxis(EAxis::X);
	
	// Pawn(Player) 위치
	FVector PawnLocation = Pawn->GetActorLocation();

	// Build Actor가 위치할 지점 ( X, Y 만 사용 )
	FVector TargetLocation = PawnLocation + (FowardDirection * PlaceDistance);

	// Build Actor Trace 지점들
	FVector TopPoint = FVector(TargetLocation.X, TargetLocation.Y, PawnLocation.Z + RangeUpHeight);
	FVector BottomPoint = FVector(TargetLocation.X, TargetLocation.Y, PawnLocation.Z - RangeDownHeight);

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
	const bool IsHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, HitParams);

	if (false == IsHit)
	{
		OutHit.ImpactPoint = FVector(TargetLocation.X, TargetLocation.Y, PawnLocation.Z);
		OutHit.ImpactNormal = IsDown ? FVector::UpVector : (FVector::UpVector * -1.f);
		
		return false;
	}

	OutHit = Hit;

	return true;
}

void UDC1BuildPlacementComponent::ApplyPreviewTransform(const FHitResult& InHit)
{
	FQuat SlopeQuat = FQuat::FindBetweenNormals(FVector::UpVector, InHit.ImpactNormal);
}
