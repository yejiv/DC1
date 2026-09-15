// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Build/DC1BuildPlacementComponent.h"
#include "Actor/BuildActor/DC1BuildActor.h"

#include "Define/DC1Helper.h"
#include "Define/DC1Structs.h"
#include "Define/DC1Stats.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Character/Player/DC1PlayerController.h"
#include "Tags/Input/DC1GameplayTag_Input.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "System/DC1AssetManager.h"
#include "Data/DA/Input/DC1InputData.h"

#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"


UDC1BuildPlacementComponent::UDC1BuildPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	SetIsReplicatedByDefault(true);
}

const float UDC1BuildPlacementComponent::GetMaxPlaceDistance() const
{
	return (true == IsSnapping || true == IsAutoSnapped) ? MaxPlaceDistance : PlaceDistance;
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

	//if (true == IsSnapping)
	//{
	//	DrawDebugSphere(GetWorld(), SnapSocketLocationWorld, 12.f, 12, FColor::Yellow);
	//	DrawDebugDirectionalArrow(GetWorld(), SnapSocketLocationWorld,
	//		SnapSocketLocationWorld + (SnapSocketNormalWorld * 80.f), 16.f, FColor::Yellow);		// 슬라이드 축
	//	DrawDebugLine(GetWorld(), SnapSocketLocationWorld, PreviewActor->GetActorLocation(), FColor::Yellow);
	//}
	//else
	//{
	//	FDC1BuildSnapSocket Debug;
	//	const ADC1BuildActor* Target = nullptr;
	//	if (true == FindSnapFace(Debug, Target))
	//	{
	//		DrawDebugSphere(GetWorld(), Debug.SocketTransform.GetLocation(), 12.f, 12, FColor::Cyan);
	//		DrawDebugDirectionalArrow(GetWorld(), Debug.SocketTransform.GetLocation(),
	//			Debug.SocketTransform.GetLocation() + (Debug.SocketTransform.GetUnitAxis(EAxis::X) * 60.f), 16.f, FColor::Cyan);
	//	}
	//}


	FDC1BuildSnapPair SnapPair;
	IsAutoSnapped = PreviewActor->GetIsAutoSnap() && FindSnapPair(SnapPair);

	if (true == IsAutoSnapped)
	{
		PreviewActor->SetActorTransform(MakeSnapTransform(SnapPair));
	}
	else
	{
		FHitResult SurfaceHit;
		TraceSurface(SurfaceHit);

		PreviewActor->SetActorTransform(PreviewActor->ComputePlaceTransform(SurfaceHit, GetPlaceYaw()));
	}

	PreviewActor->SetCheckSurface(IsAutoSnapped);
	CanBuilding = PreviewActor->UpdatePreview(IsInPlaceRange(PreviewActor->GetActorLocation()));
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

	PreviewActor = SpawnBuildActor(*BuildRow, FTransform::Identity, true);
	if (false == ::IsValid(PreviewActor))
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

bool UDC1BuildPlacementComponent::IsInPlaceRange(const FVector& InLocation) const
{
	const APawn* Pawn = GetOwnerPawn();
	if (false == ::IsValid(Pawn))
	{
		return false;
	}

	float Distance = FVector::Dist2D(Pawn->GetActorLocation(), InLocation);

	return Distance <= GetMaxPlaceDistance();
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
	ClearSnap();
}

void UDC1BuildPlacementComponent::ConfirmPlacement()
{
	if (false == ::IsValid(PreviewActor) || false == CanBuilding)
	{
		UE_LOG(LogTemp, Warning, TEXT("ConfirmPlacement :: CanBuilding false"));
		return;
	}

	Server_PlaceBuild(CurrentBuildTag, PreviewActor->GetActorTransform());
	ChangeSnapConfirmTarget();
}

void UDC1BuildPlacementComponent::Server_PlaceBuild_Implementation(const FGameplayTag& InBuildTag, const FTransform& InTransform)
{
	const FDC1BuildRow* BuildRow = FindBuildRow(InBuildTag);
	if (nullptr == BuildRow)
	{
		return;
	}

	const APawn* Pawn = GetOwnerPawn();
	if (false == ::IsValid(Pawn))
	{
		return;
	}

	const FVector PawnLocation = Pawn->GetActorLocation();
	const FVector ActorLocation = InTransform.GetLocation();

	const float Distance = FVector::Dist2D(PawnLocation, ActorLocation);

	// 서버 검증 여유 수치
	if (Distance > ( MaxPlaceDistance * DistanceMarginFactor ))
	{
		UE_LOG(LogTemp, Warning, TEXT("Server_PlaceBuild :: Too Far : %.1f"), Distance);
		return;
	}

	// TODO Cost 감소
	// ..

	SpawnBuildActor(*BuildRow, InTransform, false);
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

ADC1BuildActor* UDC1BuildPlacementComponent::SpawnBuildActor(const FDC1BuildRow& InRow, const FTransform& InTransform, bool InIsPreview)
{
	UWorld* World = GetWorld();
	if (false == ::IsValid(World))
	{
		return nullptr;
	}

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
	ADC1BuildActor* NewBuildActor = World->SpawnActorDeferred<ADC1BuildActor>(LoadedClass, InTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (false == ::IsValid(NewBuildActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cant Spawn Build Actor : %s"), *(InRow.BuildTag).ToString());
		return nullptr;
	}

	if (true == InIsPreview)
	{
		NewBuildActor->InitPreview(MakeBuildActorData(InRow));
	}
	else
	{
		NewBuildActor->InitBuild(MakeBuildActorData(InRow));
	}

	NewBuildActor->FinishSpawning(InTransform);

	return NewBuildActor;
}

bool UDC1BuildPlacementComponent::TraceSurface(FHitResult& OutHit) const
{
	const UWorld* World = GetWorld();
	if (false == ::IsValid(World))
	{
		return false;
	}

	const APawn* Pawn = GetOwnerPawn();
	if (false == ::IsValid(Pawn))
	{
		return false;
	}

	// Build Actor가 위치할 지점 ( X, Y 만 사용 )
	FVector TargetLocation = GetPlacePoint(Pawn);

	// Pawn(Player) 위치
	FVector PawnLocation = Pawn->GetActorLocation();
	const float PawnFootZ = PawnLocation.Z - Pawn->GetSimpleCollisionHalfHeight();


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
	const bool IsHit = World->LineTraceSingleByChannel(Hit, Start, End, static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_BuildSurface), HitParams);

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

	if (const UInputAction* SnapAction = InputData->FindInputActionbyTag(DC1InputBuildTags::Input_Build_Snap))
	{
		EnhancedInputComponent->BindAction(SnapAction, ETriggerEvent::Started, this, &ThisClass::OnSnapPressed);
		EnhancedInputComponent->BindAction(SnapAction, ETriggerEvent::Completed, this, &ThisClass::OnSnapReleased);
	}
}

void UDC1BuildPlacementComponent::OnRotateInput(const FInputActionValue& Value)
{
	if (false == ::IsValid(PreviewActor))
	{
		return;
	}

	const float InYaw = Value.Get<float>();

	const float Step = (true == IsSnapping || true == IsAutoSnapped) ? PreviewActor->GetSnappedRotationStep() : RotationStep;

	CurrentYaw = FMath::UnwindDegrees(CurrentYaw + (InYaw * Step));
}

void UDC1BuildPlacementComponent::OnSnapPressed()
{
	if (false == ::IsValid(PreviewActor) || true == PreviewActor->GetIsAutoSnap())
	{
		return;
	}

	FDC1BuildSnapPair SnapPair;
	if (false == FindSnapPair(SnapPair))
	{
		return;
	}

	SnapSocketLocationWorld = SnapPair.TargetSocket.SocketTransform.GetLocation();
	SnapSocketNormalWorld = SnapPair.TargetSocket.SocketTransform.GetUnitAxis(EAxis::X);
	SnapBaseYaw = SnapPair.TargetActor->GetActorRotation().Yaw;

	const FQuat SnapQuat = FRotator(0.f, SnapBaseYaw, 0.f).Quaternion();
	SnapMinDistance = PreviewActor->GetExtentCenterToDirection(SnapSocketNormalWorld, SnapQuat) + SnapContactMargin;

	SnapTarget = SnapPair.TargetActor;

	CurrentYaw = 0.f;
	IsSnapping = true;
}

void UDC1BuildPlacementComponent::OnSnapReleased()
{
	if (false == IsSnapping)
	{
		return;
	}

	const float LastYaw = GetPlaceYaw();

	ClearSnap();

	CurrentYaw = FMath::UnwindDegrees(LastYaw - GetControlYaw());
}

void UDC1BuildPlacementComponent::ClearSnap()
{
	IsSnapping = false;
	SnapTarget = nullptr;
	SnapSocketLocationWorld = FVector::ZeroVector;
	SnapSocketNormalWorld = FVector::ZeroVector;
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

FVector UDC1BuildPlacementComponent::GetAimPoint(const APawn* InOwnerPawn) const
{
	// 컨트롤러가 바라보는 방향
	FVector FowardDirection = FRotationMatrix(FRotator(0.f, GetControlYaw(), 0.f)).GetUnitAxis(EAxis::X);

	// Pawn(Player) 위치
	FVector PawnLocation = InOwnerPawn->GetActorLocation();

	// Build Actor가 위치할 지점 ( X, Y 만 사용 )
	FVector AimPoint = PawnLocation + (FowardDirection * PlaceDistance);

	return AimPoint;
}

FVector UDC1BuildPlacementComponent::GetPlacePoint(const APawn* InOwnerPawn) const
{
	const FVector AimPoint = GetAimPoint(InOwnerPawn);

	if (false == IsSnapping)
	{
		return AimPoint;
	}

	const float SnapDot = FVector::DotProduct(AimPoint - SnapSocketLocationWorld, SnapSocketNormalWorld);

	const float Distance = FMath::Max(SnapDot, SnapMinDistance);

	return SnapSocketLocationWorld + (SnapSocketNormalWorld * Distance);
}

bool UDC1BuildPlacementComponent::GetViewRay(FVector& OutStart, FVector& OutEnd) const
{
	const APlayerController* PC = GetOwner<APlayerController>();
	if (false == ::IsValid(PC))
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	OutStart = ViewLocation;
	OutEnd = ViewLocation + (ViewRotation.Vector() * MaxPlaceDistance);

	return true;
}

//bool UDC1BuildPlacementComponent::FindSnapFace(FDC1BuildSnapSocket& OutSnapSocket, const ADC1BuildActor*& OutTarget) const
//{
//	if (true == FindSnapFaceByRay(OutSnapSocket, OutTarget))
//	{
//		return true;
//	}
//
//	return FindSnapFaceByRange(OutSnapSocket, OutTarget);
//}
//
//bool UDC1BuildPlacementComponent::FindSnapFaceByRay(FDC1BuildSnapSocket& OutSnapSocket, const ADC1BuildActor*& OutTarget) const
//{
//	const UWorld* World = GetWorld();
//	if (false == ::IsValid(World))
//	{
//		return false;
//	}
//
//	FVector RayStart;
//	FVector RayEnd;
//	if (false == GetViewRay(RayStart, RayEnd))
//	{
//		return false;
//	}
//
//	FCollisionObjectQueryParams ObjectParams;
//	ObjectParams.AddObjectTypesToQuery(static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_Building));
//
//	FCollisionQueryParams Params;
//	Params.AddIgnoredActor(GetOwnerPawn());
//	Params.AddIgnoredActor(PreviewActor);
//
//	FHitResult Hit;
//	if (false == World->LineTraceSingleByObjectType(Hit, RayStart, RayEnd, ObjectParams, Params))
//	{
//		return false;
//	}
//
//	const ADC1BuildActor* Target = Cast<ADC1BuildActor>(Hit.GetActor());
//	if (false == ::IsValid(Target) || EDC1BuildState::Preview == Target->GetBuildState())
//	{
//		return false;
//	}
//
//	OutTarget = Target;
//
//	return Target->SelectSnapSocketByRay(RayStart, RayEnd, Params, OutSnapSocket);
//}
//
//bool UDC1BuildPlacementComponent::FindSnapFaceByRange(FDC1BuildSnapSocket& OutSnapSocket, const ADC1BuildActor*& OutTarget) const
//{
//	const APawn* Pawn = GetOwnerPawn();
//	if (false == ::IsValid(Pawn))
//	{
//		return false;
//	}
//
//	const FVector AimPoint = GetAimPoint(Pawn);
//
//	TSet<ADC1BuildActor*> Candidates;
//	FindSnapCandidates(AimPoint, Candidates);
//
//	float BestDistSquared = FLT_MAX;
//	bool IsFind = false;
//
//	for (const ADC1BuildActor* Candidate : Candidates)
//	{
//		FDC1BuildSnapSocket SnapSocket;
//		if (false == Candidate->SelectSnapSocketByPoint(AimPoint, SnapSocket))
//		{
//			continue;
//		}
//
//		const float DistSquared = FVector::DistSquaredXY(SnapSocket.SocketTransform.GetLocation(), AimPoint);
//
//		if (DistSquared < BestDistSquared)
//		{
//			BestDistSquared = DistSquared;
//			OutSnapSocket = SnapSocket;
//			OutTarget = Candidate;
//			IsFind = true;
//		}
//	}
//
//	return IsFind;
//}

void UDC1BuildPlacementComponent::FindSnapCandidates(const FVector& InAimPoint, TSet<ADC1BuildActor*>& OutCandidates) const
{
	OutCandidates.Reset();

	SCOPE_CYCLE_COUNTER(STAT_DC1_SnapOverlapQuery);
	
	const UWorld* World = GetWorld();
	if (false == ::IsValid(World))
	{
		return;
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_Building));

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwnerPawn());
	Params.AddIgnoredActor(PreviewActor);

	TArray<FOverlapResult> Overlaps;

	if (false == World->OverlapMultiByObjectType(Overlaps, InAimPoint, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(SnapSearchRadius), Params))
	{
		return;
	}

	for (const FOverlapResult Result : Overlaps)
	{
		ADC1BuildActor* OverlapActor = Cast<ADC1BuildActor>(Result.GetActor());
		if (false == ::IsValid(OverlapActor))
		{
			continue;
		}

		OutCandidates.Add(OverlapActor);
	}
}

bool UDC1BuildPlacementComponent::FindSnapPair(FDC1BuildSnapPair& OutPair) const
{
	SCOPE_CYCLE_COUNTER(STAT_DC1_FindsnapPair);

	const APawn* Pawn = GetOwnerPawn();
	if (false == ::IsValid(Pawn) || false == ::IsValid(PreviewActor))
	{
		return false;
	}

	TArray<FDC1BuildSnapSocket> PreviewSockets;
	if (false == PreviewActor->GetBuildSocketsToWorld(PreviewSockets))
	{
		return false;
	}

	TArray<FDC1BuildBox> Blockers;

	TSet<ADC1BuildActor*> RayCandidates;
	FVector RayPoint;

	if (true == FindSnapCandidateByRay(RayCandidates, RayPoint))
	{
		TSet<ADC1BuildActor*> Neighbors;
		FindSnapCandidates(RayPoint, Neighbors);

		//GatherBlockers(Neighbors, Blockers);

		if (true == FindFreeSnapPair(RayCandidates, RayPoint, PreviewSockets, Blockers, OutPair))
		{
			return true;
		}
	}

	const FVector AimPoint = GetAimPoint(Pawn);

	TSet<ADC1BuildActor*> Candidates;
	FindSnapCandidates(AimPoint, Candidates);

	GatherBlockers(Candidates, Blockers);

	return FindFreeSnapPair(Candidates, AimPoint, PreviewSockets, Blockers, OutPair);
}

bool UDC1BuildPlacementComponent::FindSnapCandidateByRay(TSet<ADC1BuildActor*>& OutCandidates, FVector& OutAimPoint) const
{
	OutCandidates.Reset();

	const UWorld* World = GetWorld();
	if (false == ::IsValid(World))
	{
		return false;
	}

	FVector RayStart;
	FVector RayEnd;
	if (false == GetViewRay(RayStart, RayEnd))
	{
		return false;
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_Building));

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwnerPawn());
	Params.AddIgnoredActor(PreviewActor);

	FHitResult Hit;
	if (false == World->LineTraceSingleByObjectType(Hit, RayStart, RayEnd, ObjectParams, Params))
	{
		return false;
	}

	ADC1BuildActor* Target = Cast<ADC1BuildActor>(Hit.GetActor());
	if (false == ::IsValid(Target) || true == Target->IsPreview())
	{
		return false;
	}

	if (false == Target->TraceCheckBox(RayStart, RayEnd, Params, SnapAimInflate, OutAimPoint))
	{
		// 박스가 메시보다 작은 경우에 월드 Hit로 대체
		OutAimPoint = Hit.ImpactPoint;
	}

	OutCandidates.Add(Target);

	return true;
}

bool UDC1BuildPlacementComponent::FindFreeSnapPair(const TSet<ADC1BuildActor*>& InCandidates, const FVector& InAimPoint, const TArray<FDC1BuildSnapSocket>& InPreviewSockets, const TArray<FDC1BuildBox>& InBlockers, FDC1BuildSnapPair& OutPair) const
{
	if (false == ::IsValid(PreviewActor))
	{
		return false;
	}

	// 검사는 뽑힌 소켓만 - 모든 소켓을 돌며 매번 검사하면 후보 * 소켓 * 프리뷰 * 이웃으로 곱해짐 ( 측정 0.85ms )
	TArray<TWeakObjectPtr<const UDC1BuildSocketComponent>> ExcludedSockets;

	// 사방이 찬 경우의 상한선
	constexpr int32 MaxTries = 8;

	for (int32 Try = 0; Try < MaxTries; ++Try)
	{
		FDC1BuildSnapPair Pair;

		if (false == FindBestSnapPair(InCandidates, InAimPoint, InPreviewSockets, ExcludedSockets, Pair))
		{
			return false;
		}

		{

			SCOPE_CYCLE_COUNTER(STAT_DC1_SnapBlockTest);
			
			if (false == PreviewActor->IsBlockedAt(MakeSnapTransform(Pair), InBlockers))
			{
				OutPair = Pair;
				return true;
			}
		}
		ExcludedSockets.Add(Pair.TargetSocket.SourceSocket);
	}

	return false;
}

bool UDC1BuildPlacementComponent::FindBestSnapPair(const TSet<ADC1BuildActor*>& InCandidates, const FVector& InAimPoint, const TArray<FDC1BuildSnapSocket>& InPreviewSockets, const TArray<TWeakObjectPtr<const UDC1BuildSocketComponent>>& InExcludedSockets, FDC1BuildSnapPair& OutPair) const
{
	float BestActorDistSquared = FLT_MAX;

	bool IsFind = false;

	for (const ADC1BuildActor* Candidate : InCandidates)
	{
		if (true == Candidate->IsPreview())
		{
			continue;
		}

		const float ActorDistSquared = FVector::DistSquared(InAimPoint, Candidate->GetCheckBoxCenter());

		if (ActorDistSquared >= BestActorDistSquared)
		{
			continue;
		}

		TArray<FDC1BuildSnapSocket> TargetSockets;
		if (false == Candidate->GetBuildSocketsToWorld(TargetSockets))
		{
			continue;
		}

		float BestScore = FLT_MAX * -1.f;
		float BestDistSquared = FLT_MAX;
		float BestFacing = FLT_MAX * -1.f;

		bool IsSocketFind = false;

		FDC1BuildSnapSocket BsetTargetSocket;
		FDC1BuildSnapSocket BestPreviewSocket;

		for (const FDC1BuildSnapSocket& TargetSocket : TargetSockets)
		{
			// 이번 프레임에 이미 판정된 소켓
			if (true == InExcludedSockets.Contains(TargetSocket.SourceSocket))
			{
				continue;
			}

			float Score = 0.f;
			float DistSquared = 0.f;
			float Facing = 0.f;

			TargetSocket.EvaluateFrom(InAimPoint, Score, DistSquared, Facing);

			if (false == FDC1BuildSnapSocket::IsBetter(Score, DistSquared, Facing, BestScore, BestDistSquared, BestFacing))
			{
				continue;
			}

			for (const FDC1BuildSnapSocket& PreviewSocket : InPreviewSockets)
			{
				if (false == TargetSocket.AcceptTypes.HasTag(PreviewSocket.SnapType))
				{
					continue;
				}

				IsSocketFind = true;

				BestScore = Score;
				BestDistSquared = DistSquared;
				BestFacing = Facing;

				BsetTargetSocket = TargetSocket;
				BestPreviewSocket = PreviewSocket;
			}

			if (false == IsSocketFind)
			{
				continue;
			}

			BestActorDistSquared = ActorDistSquared;

			FDC1BuildSnapPair SnapPair;
			SnapPair.TargetActor = Candidate;
			SnapPair.TargetSocket = BsetTargetSocket;
			SnapPair.PreviewSocket = BestPreviewSocket;

			OutPair = SnapPair;

			IsFind = true;
		}
	}

	return IsFind;
}

const FTransform UDC1BuildPlacementComponent::MakeSnapTransform(const FDC1BuildSnapPair& InPair) const
{
	// 두 소켓이 마주보게
	const FTransform Flip = FTransform(FRotator(0.f, 180.f, 0.f));
	const FTransform Desired = Flip * InPair.TargetSocket.SocketTransform;

	const FQuat DeltaQuat = Desired.GetRotation() * InPair.PreviewSocket.SocketTransform.GetRotation().Inverse();

	const FVector ActorLocation = PreviewActor->GetActorLocation();
	const FVector SocketOffset = InPair.PreviewSocket.SocketTransform.GetLocation() - ActorLocation;

	const FVector AttachAxis = InPair.TargetSocket.SocketTransform.GetUnitAxis(EAxis::X);
	const FVector Anchor = Desired.GetLocation() + (AttachAxis * SnapContactMargin);

	const FQuat BaseQuat = DeltaQuat * PreviewActor->GetActorQuat();
	const FVector BaseLocation = Anchor - DeltaQuat.RotateVector(SocketOffset);

	// 피벗 ( 박스 중심 )
	const FVector BoxOffset = PreviewActor->GetCheckBoxCenter() - ActorLocation;
	const FVector PivotWorld = BaseLocation + DeltaQuat.RotateVector(BoxOffset);

	const float SnappedYaw = FMath::GridSnap(CurrentYaw, PreviewActor->GetSnappedRotationStep());
	const FQuat ExtraQuat = FRotator(0.f, SnappedYaw, 0.f).Quaternion();

	const FQuat FinalQuat = ExtraQuat * BaseQuat;
	const FVector FinalLocation = PivotWorld + ExtraQuat.RotateVector(BaseLocation - PivotWorld);

	return FTransform(FinalQuat, FinalLocation, PreviewActor->GetActorScale3D());
}

void UDC1BuildPlacementComponent::ChangeSnapConfirmTarget()
{
	if (false == IsSnapping)
	{
		return;
	}

	const FQuat SnapQuat = FRotator(0.f, SnapBaseYaw, 0.f).Quaternion();
	SnapMinDistance = PreviewActor->GetExtentCenterToDirection(SnapSocketNormalWorld, SnapQuat);

	const FVector NewBoxCenter = PreviewActor->GetCheckBoxCenter();
	const float HalfExtent = SnapMinDistance - SnapContactMargin;

	SnapSocketLocationWorld = NewBoxCenter + (SnapSocketNormalWorld * HalfExtent);
}

void UDC1BuildPlacementComponent::GatherBlockers(const TSet<ADC1BuildActor*>& InNeighbors, TArray<FDC1BuildBox>& OutBlockers) const
{
	OutBlockers.Reset();

	for (const ADC1BuildActor* Neighbor : InNeighbors)
	{
		if (true == ::IsValid(Neighbor))
		{
			FDC1BuildBox NeighborBox;
			if (true == Neighbor->GetCheckBoxWorld(NeighborBox))
			{
				OutBlockers.Add(NeighborBox);
			}
		}
	}


}
