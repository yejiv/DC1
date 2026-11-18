// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Build/DC1BuildPlacementComponent.h"
#include "Actor/BuildActor/DC1BuildActor.h"

#include "Component/Build/Placement/DC1BuildSnapSolver.h"

#include "Define/DC1Helper.h"
#include "Define/DC1BuildSupportHelper.h"
#include "Define/DC1CraftHelper.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Character/Player/DC1PlayerController.h"
#include "Character/Player/DC1PlayerState.h"
#include "Character/Player/DC1Player.h"
#include "Tags/Input/DC1GameplayTag_Input.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "System/DC1AssetManager.h"
#include "Data/DA/Input/DC1InputData.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

#include "Subsystem/DC1BuildDestroySubsystem.h"
#include "Character/DC1Character.h"

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

	SnapSolver = NewObject<UDC1BuildSnapSolver>(this);

	CacheBuildRows();
	SetUpBuildInput();
}

void UDC1BuildPlacementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelPlacement();
	CancelDismantle();

	Super::EndPlay(EndPlayReason);
}

void UDC1BuildPlacementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (true == IsDismantling)
	{
		UpdateDismantleTarget();
		return;
	}

	if (false == ::IsValid(PreviewActor))
	{
		return;
	}

	FTransform SnapTransform;

	IsAutoSnapped = PreviewActor->GetIsAutoSnap() && ::IsValid(SnapSolver) && SnapSolver->FindSnapTransform(MakeSnapInput(), SnapTransform);

	if (true == IsAutoSnapped)
	{
		PreviewActor->SetActorTransform(SnapTransform);
	}
	else
	{
		FHitResult SurfaceHit;
		TraceSurface(SurfaceHit);

		SurfaceHit.ImpactPoint.Z += ComputePillarHeightOffset();

		PreviewActor->SetActorTransform(PreviewActor->ComputePlaceTransform(SurfaceHit, GetPlaceYaw()));
	}

	PreviewActor->SetCheckSurface(IsAutoSnapped);

	bool IsInRange = IsInBuildRange(PreviewActor, MaxDistance);

	// 설치 불가 broadcast
	SetBlockReason(PreviewActor->UpdatePreview(IsInRange, GetOwner<APlayerController>()->PlayerState));
}

bool UDC1BuildPlacementComponent::StartPlacement(const FGameplayTag& InBuildTag)
{
	if (true == IsOwnerDownOrDead())
	{
		return false;
	}

	const FDC1BuildRow* BuildRow = FindBuildRow(InBuildTag);
	if (nullptr == BuildRow)
	{
		return false;
	}

	const APlayerController* PC = GetOwner<APlayerController>();
	const ADC1PlayerState* PlayerState = (true == ::IsValid(PC)) ? PC->GetPlayerState<ADC1PlayerState>() : nullptr;
	// 재료 확인
	if (false == DC1CraftHelper::HasCraftCosts(PlayerState, BuildRow->BuildCosts))
	{
		return false;
	}
	// 파괴 모드 취소
	CancelDismantle();
	// 배치 중 이었다면 빌드 모드 취소
	if (true == ::IsValid(PreviewActor))
	{
		CancelPlacement();
	}

	PreviewActor = SpawnBuildActor(*BuildRow, FTransform::Identity, true);
	if (false == ::IsValid(PreviewActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildPlacementComponent :: Can't Spawn Build Actor : %s"), *InBuildTag.ToString());
		return false;
	}
	// 프리뷰는 복제 X, Only 클라
	PreviewActor->SetReplicates(false);

	CurrentPlaceDistance = ComputePlaceDistance();

	CurrentBuildTag = InBuildTag;
	// 모드 전환용
	LastBuildTag = InBuildTag;
	CurrentYaw = 0.f;

	SetComponentTickEnabled(true);

	// Build IMC 추가, State_Build Tag 추가, InteractComp 비활성화
	SetBuildMode(true);

	return true;
}

bool UDC1BuildPlacementComponent::StartDismantle()
{
	if (true == IsOwnerDownOrDead())
	{
		return false;
	}

	CancelPlacement();

	IsDismantling = true;

	SetBlockReason(EDC1BuildBlockReason::NoDismantleTarget);

	SetComponentTickEnabled(true);
	SetBuildMode(true);

	return true;
}

void UDC1BuildPlacementComponent::CancelBuildMode()
{
	CancelPlacement();
	CancelDismantle();
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
	SetBlockReason(EDC1BuildBlockReason::None);

	// Build IMC 제거, State_Build Tag 제거, InteractComp 활성화
	SetBuildMode(false);

	if (true == ::IsValid(SnapSolver))
	{
		SnapSolver->ClearSnap();
	}
}

void UDC1BuildPlacementComponent::ConfirmPlacement()
{
	if (false == ::IsValid(PreviewActor) || EDC1BuildBlockReason::None != CurrentBlockReason)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildPlacementComponent :: ConfirmPlacement - CanBuilding false"));

		if (true == ::IsValid(PreviewActor))
		{
			UGameplayStatics::PlaySound2D(this, PlaceErrorSound);
		}

		return;
	}

	Server_PlaceBuild(CurrentBuildTag, PreviewActor->GetActorTransform());

	if (true == ::IsValid(SnapSolver))
	{
		SnapSolver->OnPlacementConfirmed(PreviewActor);
	}
}

void UDC1BuildPlacementComponent::CancelDismantle()
{
	SetComponentTickEnabled(false);

	ADC1BuildActor* Target = DismantleTarget.Get();
	if (true == ::IsValid(Target))
	{
		Target->SetDismantleHighlight(false);
	}

	DismantleTarget.Reset();
	IsDismantling = false;

	SetBlockReason(EDC1BuildBlockReason::None);

	OnDismantleTargetChanged.Broadcast(nullptr);

	SetBuildMode(false);
}

void UDC1BuildPlacementComponent::ConfirmDismantle()
{
	ADC1BuildActor* CurrentTarget = DismantleTarget.Get();
	if (false == ::IsValid(CurrentTarget))
	{
		return;
	}

	Server_DismantleBuild(CurrentTarget);
}

void UDC1BuildPlacementComponent::Server_PlaceBuild_Implementation(const FGameplayTag& InBuildTag, const FTransform& InTransform)
{
	if (true == IsOwnerDownOrDead())
	{
		return;
	}

	const FDC1BuildRow* BuildRow = FindBuildRow(InBuildTag);
	if (nullptr == BuildRow)
	{
		return;
	}

	APlayerController* PC = GetOwner<APlayerController>();
	if (false == ::IsValid(PC))
	{
		return;
	}

	ADC1PlayerState* PlayerState = PC->GetPlayerState<ADC1PlayerState>();
	if (false == DC1CraftHelper::HasCraftCosts(PlayerState, BuildRow->BuildCosts))
	{
		return;
	}

	ADC1BuildActor* NewBuild = SpawnBuildActor(*BuildRow, InTransform, false);
	if (false == ::IsValid(NewBuild))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildPlacementComponent :: Server_PlaceBuild - Failed Spawn Build Actor"));
		return;
	}

	// 서버 검증 여유 수치
	if (false == IsInBuildRange(NewBuild, MaxDistance, DistanceMarginFactor) || false == DC1BuildSupportHelper::HasSupport(NewBuild)
		|| false == NewBuild->CanPlaceAt(PC->PlayerState))
	{
		NewBuild->Destroy();
		return;
	}

	NewBuild->OnPlaced(PC->PlayerState);

	// 설치음 - 설치한 플레이어 경유로 전원 재생
	if (ADC1Player* Builder = PC->GetPawn<ADC1Player>())
	{
		Builder->PlayWorldSound(NewBuild->GetPlaceSound(), NewBuild->GetActorLocation());
	}

	DC1CraftHelper::RemoveCraftCosts(PlayerState, BuildRow->BuildCosts);
}

void UDC1BuildPlacementComponent::Server_DismantleBuild_Implementation(ADC1BuildActor* InTarget)
{
	if (true == IsOwnerDownOrDead())
	{
		return;
	}

	if (false == ::IsValid(InTarget) || true == InTarget->IsPreview())
	{
		return;
	}

	if (false == IsInBuildRange(InTarget, MaxDistance, DistanceMarginFactor))
	{
		return;
	}

	UDC1BuildDestroySubsystem* DestroySubsystem = GetWorld()->GetSubsystem<UDC1BuildDestroySubsystem>();
	if (true == ::IsValid(DestroySubsystem))
	{
		APlayerController* PC = GetOwner<APlayerController>();
		APlayerState* PS = (nullptr != PC) ? PC->PlayerState : nullptr;

		// 철거음 - 철거한 플레이어 경유로 전원 재생 ( RequestDestroy 전에 위치를 잡아둔다 )
		const FVector DismantleLocation = InTarget->GetActorLocation();
		USoundBase* TargetDismantleSound = (nullptr != InTarget->GetDismantleSound()) ? InTarget->GetDismantleSound() : DismantleSound.Get();
		const bool WasDestroying = InTarget->IsInDestroySequence();

		DestroySubsystem->RequestDestroy(InTarget, PS, EDC1BuildRefundMode::ToInstigator);

		ADC1Player* Dismantler = (nullptr != PC) ? PC->GetPawn<ADC1Player>() : nullptr;
		if (false == WasDestroying && nullptr != Dismantler)
		{
			Dismantler->PlayWorldSound(TargetDismantleSound, DismantleLocation);
		}
	}
}

void UDC1BuildPlacementComponent::CacheBuildRows()
{
	TArray<FDC1BuildRow> AllRows;
	if (false == DC1DataTableHelper::GetAllRows<FDC1BuildRow>(DC1DataTableTags::DataTable_UI_Build, AllRows))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildPlacementComponent :: Build DataTable is NULL"));
		return;
	}

	BuildRowsByTag.Empty();

	for (const FDC1BuildRow& Row : AllRows)
	{
		const FGameplayTag& BuildTag = Row.BuildTag;
		if (false == BuildTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("UDC1BuildPlacementComponent :: Build Tag is Invalid"));
			continue;
		}

		if (nullptr != BuildRowsByTag.Find(BuildTag))
		{
			UE_LOG(LogTemp, Warning, TEXT("UDC1BuildPlacementComponent :: Duplication BuildTag"));
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
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildPlacementComponent :: Can't Find BuildRow : %s"), *InBuildTag.ToString());
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
	Data.WorkTypeTag = InRow.WorkTypeTag;

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
	if (true == InRow.BuildActorClass.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildPlacementComponent :: Empty Build Actor Class : %s"), *(InRow.BuildTag).ToString());
		return nullptr;
	}

	UClass* LoadedClass = InRow.BuildActorClass.LoadSynchronous();
	if (false == ::IsValid(LoadedClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildPlacementComponent :: Failed Load Build Actor Class : %s"), *(InRow.BuildTag).ToString());
		return nullptr;
	}

	// State 세팅을 위해 SpawnActorDeferred | AlwaysSpawn - 스폰 위치에 다른 충돌체가 있어도 스폰
	ADC1BuildActor* NewBuildActor = World->SpawnActorDeferred<ADC1BuildActor>(LoadedClass, InTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (false == ::IsValid(NewBuildActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildPlacementComponent :: Can't Spawn Build Actor : %s"), *(InRow.BuildTag).ToString());
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
		EnhancedInputComponent->BindAction(ConfirmAction, ETriggerEvent::Started, this, &ThisClass::OnConfirmInput);
	}

	if (const UInputAction* CancelAction = InputData->FindInputActionbyTag(DC1InputBuildTags::Input_Build_Cancel))
	{
		EnhancedInputComponent->BindAction(CancelAction, ETriggerEvent::Started, this, &ThisClass::OnBackInput);
	}

	if (const UInputAction* ExitAction = InputData->FindInputActionbyTag(DC1InputBuildTags::Input_Build_Exit))
	{
		EnhancedInputComponent->BindAction(ExitAction, ETriggerEvent::Started, this, &ThisClass::OnExitInput);
	}

	if (const UInputAction* ToggleModeAction = InputData->FindInputActionbyTag(DC1InputBuildTags::Input_Build_ToggleMode))
	{
		EnhancedInputComponent->BindAction(ToggleModeAction, ETriggerEvent::Started, this, &ThisClass::OnToggleModeInput);
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

void UDC1BuildPlacementComponent::OnConfirmInput()
{
	if (true == IsDismantling)
	{
		ConfirmDismantle();
		return;
	}

	ConfirmPlacement();
}

void UDC1BuildPlacementComponent::OnBackInput()
{
	UGameplayStatics::PlaySound2D(this, CancelSound);
	ReturnToBuildMenu();
}

void UDC1BuildPlacementComponent::OnExitInput()
{
	UGameplayStatics::PlaySound2D(this, CancelSound);
	CancelBuildMode();
}

void UDC1BuildPlacementComponent::OnToggleModeInput()
{
	if (true == IsDismantling)
	{
		if (false == StartPlacement(LastBuildTag))
		{
			ReturnToBuildMenu();
		}

		return;
	}

	StartDismantle();
}

void UDC1BuildPlacementComponent::ReturnToBuildMenu()
{
	const FGameplayTag TabTag = MakeBuildTabTag(LastBuildTag);

	CancelBuildMode();

	ADC1PlayerController* PC = GetOwner<ADC1PlayerController>();
	if (true == ::IsValid(PC))
	{
		PC->OpenBuildMenu(TabTag);
	}
}

FGameplayTag UDC1BuildPlacementComponent::MakeBuildTabTag(const FGameplayTag& InBuildTag)
{
	if (false == InBuildTag.IsValid())
	{
		return FGameplayTag::EmptyTag;
	}

	return InBuildTag.RequestDirectParent().RequestDirectParent();
}

void UDC1BuildPlacementComponent::OnRotateInput(const FInputActionValue& Value)
{
	if (false == ::IsValid(PreviewActor))
	{
		return;
	}

	const float InYaw = Value.Get<float>();

	const bool IsSnapping = ::IsValid(SnapSolver) && SnapSolver->GetIsSnapping();

	const float Step = (true == IsSnapping || true == IsAutoSnapped) ? PreviewActor->GetSnappedRotationStep() : RotationStep;

	CurrentYaw = FMath::UnwindDegrees(CurrentYaw + (InYaw * Step));
}

void UDC1BuildPlacementComponent::OnSnapPressed()
{
	if (false == ::IsValid(PreviewActor) || true == PreviewActor->GetIsAutoSnap() || false == ::IsValid(SnapSolver))
	{
		return;
	}

	if (false == SnapSolver->BeginManualSnap(MakeSnapInput()))
	{
		return;
	}

	CurrentYaw = 0.f;
}

void UDC1BuildPlacementComponent::OnSnapReleased()
{
	if (false == ::IsValid(SnapSolver) || false == SnapSolver->GetIsSnapping())
	{
		return;
	}

	const float LastYaw = GetPlaceYaw();

	SnapSolver->ClearSnap();

	CurrentYaw = FMath::UnwindDegrees(LastYaw - GetControlYaw());
}

void UDC1BuildPlacementComponent::UpdateDismantleTarget()
{

	FVector RayStart;
	FVector RayEnd;
	FHitResult Hit;
	ADC1BuildActor* HitTarget = TraceBuildActorByView(RayStart, RayEnd, Hit);

	if (false == ::IsValid(HitTarget) || true == HitTarget->IsInDestroySequence() || false == IsInBuildRange(HitTarget, MaxDistance))
	{
		HitTarget = nullptr;
	}

	ADC1BuildActor* PrevTarget = DismantleTarget.Get();
	if (HitTarget == PrevTarget && false == DismantleTarget.IsStale())
	{
		return;
	}

	if (true == ::IsValid(PrevTarget))
	{
		PrevTarget->SetDismantleHighlight(false);
	}

	if (true == ::IsValid(HitTarget))
	{
		HitTarget->SetDismantleHighlight(true);
	}

	DismantleTarget = HitTarget;

	SetBlockReason((true == ::IsValid(HitTarget)) ? EDC1BuildBlockReason::None : EDC1BuildBlockReason::NoDismantleTarget);

	OnDismantleTargetChanged.Broadcast(HitTarget);
}

void UDC1BuildPlacementComponent::SetBuildMode(bool InEnabled)
{
	ADC1PlayerController* PC = GetOwner<ADC1PlayerController>();
	if (true == ::IsValid(PC))
	{
		PC->SetIMCEnabled(DC1IMCTags::IMC_Build, InEnabled);
		PC->SetBuildMode(InEnabled);
	}

	RefreshBuildMode();
}

void UDC1BuildPlacementComponent::RefreshBuildMode()
{
	EDC1BuildMode NewMode = EDC1BuildMode::Default;

	if (true == IsDismantling)
	{
		NewMode = EDC1BuildMode::Dismantle;
	}
	else if (true == ::IsValid(PreviewActor))
	{
		NewMode = EDC1BuildMode::Placement;
	}

	if (CurrentMode == NewMode)
	{
		return;
	}

	CurrentMode = NewMode;

	OnBuildModeChanged.Broadcast(NewMode, CurrentBuildTag);
}

void UDC1BuildPlacementComponent::SetBlockReason(EDC1BuildBlockReason InReason)
{
	if (CurrentBlockReason == InReason)
	{
		return;
	}

	CurrentBlockReason = InReason;

	OnBlockReasonChanged.Broadcast(InReason);
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
	const bool IsSnapping = ::IsValid(SnapSolver) && SnapSolver->GetIsSnapping();

	float BaseYaw = IsSnapping ? SnapSolver->GetSnapBaseYaw() : GetControlYaw();

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
	FVector AimPoint = PawnLocation + (FowardDirection * ComputeAimDistance());

	return AimPoint;
}

FVector UDC1BuildPlacementComponent::GetPlacePoint(const APawn* InOwnerPawn) const
{
	const FVector AimPoint = GetAimPoint(InOwnerPawn);

	if (false == ::IsValid(SnapSolver))
	{
		return AimPoint;
	}

	// 스냅 중이 아니면 AimPoint를 그대로 돌려준다
	return SnapSolver->GetSnappedPlacePoint(AimPoint);
}

FDC1BuildSnapInput UDC1BuildPlacementComponent::MakeSnapInput() const
{
	FDC1BuildSnapInput Input;

	const APawn* Pawn = GetOwnerPawn();

	Input.Preview = PreviewActor;
	Input.Pawn = Pawn;
	Input.ControlYaw = GetControlYaw();
	Input.CurrentYaw = CurrentYaw;
	Input.SearchRadius = SnapSearchRadius;
	Input.AimInflate = SnapAimInflate;

	if (true == ::IsValid(Pawn))
	{
		Input.AimPoint = GetAimPoint(Pawn);
	}

	FHitResult Hit;
	Input.RayTarget = TraceBuildActorByView(Input.RayStart, Input.RayEnd, Hit);
	Input.RayImpactPoint = Hit.ImpactPoint;

	return Input;
}

bool UDC1BuildPlacementComponent::GetViewRay(FVector& OutStart, FVector& OutEnd) const
{
	const APlayerController* PC = GetOwner<APlayerController>();
	const APawn* Pawn = GetOwnerPawn();
	if (false == ::IsValid(PC) || false == ::IsValid(Pawn))
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const float CameraToPawn = FVector::Dist(ViewLocation, Pawn->GetActorLocation());

	OutStart = ViewLocation;
	OutEnd = ViewLocation + (ViewRotation.Vector() * (CameraToPawn + MaxDistance));

	return true;
}
bool UDC1BuildPlacementComponent::IsInBuildRange(const ADC1BuildActor* InBuild, const float InMaxDistance, const float InMarginFactor) const
{
	const APawn* Pawn = GetOwnerPawn();
	if (false == ::IsValid(Pawn) || false == ::IsValid(InBuild))
	{
		return false;
	}

	FDC1BuildBox Box;
	if (false == InBuild->GetCheckBoxWorld(Box))
	{
		return false;
	}

	const FVector PawnWorldLocation = Pawn->GetActorLocation();

	const FVector LocalLocation = Box.Transform.InverseTransformPositionNoScale(PawnWorldLocation);

	const FVector Clamp = FBox(Box.Extent * -1.f, Box.Extent).GetClosestPointTo(LocalLocation);

	const FVector Closet = Box.Transform.TransformPositionNoScale(Clamp);

	return FVector::Dist2D(PawnWorldLocation, Closet) <= (InMaxDistance * InMarginFactor);
}

ADC1BuildActor* UDC1BuildPlacementComponent::TraceBuildActorByView(FVector& OutRayStart, FVector& OutRayEnd, FHitResult& OutHit) const
{
	const UWorld* World = GetWorld();
	if (false == ::IsValid(World) || false == GetViewRay(OutRayStart, OutRayEnd))
	{
		return nullptr;
	}

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_Building));

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwnerPawn());
	Params.AddIgnoredActor(PreviewActor);

	if (false == World->LineTraceSingleByObjectType(OutHit, OutRayStart, OutRayEnd, ObjectParams, Params))
	{
		return nullptr;
	}

	ADC1BuildActor* Target = Cast<ADC1BuildActor>(OutHit.GetActor());
	if (false == ::IsValid(Target) || true == Target->IsPreview())
	{
		return nullptr;
	}

	return Target;
}

float UDC1BuildPlacementComponent::ComputePillarHeightOffset() const
{
	if (false == ::IsValid(PreviewActor) || false == PreviewActor->GetHasPillar())
	{
		return 0.f;
	}

	const APawn* Pawn = GetOwnerPawn();
	if (false == ::IsValid(Pawn))
	{
		return 0.f;
	}

	float Pitch = FMath::Min(FRotator::NormalizeAxis(Pawn->GetControlRotation().Pitch), MaxPillarDegree);

	if (Pitch < 0.f)
	{
		return 0.f;
	}

	return CurrentPlaceDistance * FMath::Tan(FMath::DegreesToRadians(Pitch));
}

bool UDC1BuildPlacementComponent::IsOwnerDownOrDead() const
{
	APlayerController* PC = GetOwner<APlayerController>();
	if (false == ::IsValid(PC))
	{
		return false;
	}
	
	ADC1Character* Owner = PC->GetPawn<ADC1Character>();
	
	if (true == ::IsValid(Owner) && true == Owner->IsDownOrDead())
	{
		return true;
	}

	return false;
}

float UDC1BuildPlacementComponent::ComputePlaceDistance() const
{
	const APawn* Pawn = GetOwnerPawn();
	if (false == ::IsValid(Pawn) || false == ::IsValid(PreviewActor))
	{
		return 0.f;
	}

	return PreviewActor->GetMaxCheckRadius2D() + Pawn->GetSimpleCollisionRadius() + PlaceMargin;
}

float UDC1BuildPlacementComponent::ComputeAimDistance() const
{
	if (false == ::IsValid(PreviewActor) || true == PreviewActor->GetHasPillar())
	{
		return CurrentPlaceDistance;
	}

	const APawn* Pawn = GetOwnerPawn();
	if (false == ::IsValid(Pawn))
	{
		return CurrentPlaceDistance;
	}

	float Pitch = FRotator::NormalizeAxis(Pawn->GetControlRotation().Pitch);

	float FarDistance = FMath::Max(MaxDistance, CurrentPlaceDistance);

	return FMath::GetMappedRangeValueClamped(FVector2f(-30.f, MaxFarDegree), FVector2f(CurrentPlaceDistance, FarDistance), Pitch);;
}
