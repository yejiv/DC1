// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Interact/DC1InteractBuildActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Define/DC1Enums.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Character/Player/DC1PlayerController.h"
#include "Component/Interact/DC1InteractComponent.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystem/Attribute/DC1PalAttributeSet.h"

#include "Tags/State/DC1GameplayTag_State.h"
#include "Tags/Player/DC1GameplayTag_Player.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "Data/DA/Pal/DC1PalData.h"

#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"

#include "Components/WidgetComponent.h"
#include "UI/World/BuildInfo/DC1BuildInfoWidget.h"

#include "Net/UnrealNetwork.h"

// TEMP v6 - DebugAssignNearbyPals ( v7에서 함께 삭제 )
#include "EngineUtils.h"

ADC1InteractBuildActor::ADC1InteractBuildActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	InteractCollider = CreateDefaultSubobject<UBoxComponent>("InteractCollider");

	InteractCollider->SetCollisionProfileName(DC1CollisionProfile::Interaction);
	InteractCollider->SetupAttachment(BuildMesh);
	InteractCollider->SetGenerateOverlapEvents(true);

	ConstructionOptions.Emplace(EDC1InteractKey::F, NSLOCTEXT("Interact", "Build", "건설"), EDC1InteractInputType::HoldAbility, DC1PlayerTags::Ability_Player_BuildWork);
	ConstructionOptions.Emplace(EDC1InteractKey::C, NSLOCTEXT("Interact", "Cancel", "취소"), EDC1InteractInputType::HoldConfirm, FGameplayTag::EmptyTag);

	InfoWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("InfoWidgetComponent");
	InfoWidgetComponent->SetupAttachment(BuildMesh);
	InfoWidgetComponent->SetUsingAbsoluteLocation(false);
	InfoWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	InfoWidgetComponent->SetDrawAtDesiredSize(true);
	InfoWidgetComponent->SetPivot({ 0.5f, 0.5f });
	InfoWidgetComponent->SetVisibility(false);
	InfoWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InfoWidgetComponent->SetInitialSharedLayerName("InteractInfo");
	InfoWidgetComponent->SetInitialLayerZOrder(-200);
}

float ADC1InteractBuildActor::GetRemainingWork() const
{
	return FMath::Max(0.f, BuildActorData.RequiredWork * ( 1.f - BuildProgress));
}

bool ADC1InteractBuildActor::IsPalWorkerAssigned(const ADC1PalCharacter* InPal) const
{
	return PalWorkers.Contains(InPal);
}

FGameplayTag ADC1InteractBuildActor::GetRequiredWorkTag() const
{
	return IsUnderConstruction() ? DC1PalTags::Pal_Work_Handiwork : FGameplayTag::EmptyTag;
}

void ADC1InteractBuildActor::Tick(float DeltaTime)
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

	// 건설 / 생산 처리
	if (true == Workers.IsEmpty())
	{
		SetCurrentWorkSpeed(0.f);
		SetActorTickEnabled(false);
		return;
	}

	if (false == HasAuthority())
	{
		return;
	}

	const float TotalWorkSpeed = ComputeTotalWorkSpeed();
	SetCurrentWorkSpeed(TotalWorkSpeed);

	if (TotalWorkSpeed > 0.f)
	{
		OnWorkTick(TotalWorkSpeed * DeltaTime);
	}
}

void ADC1InteractBuildActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, BuildProgress);
	DOREPLIFETIME(ThisClass, CurrentWorkSpeed);
	DOREPLIFETIME(ThisClass, PalWorkers);
}

UPrimitiveComponent* ADC1InteractBuildActor::GetInteractCollider() const
{
	if (false == ::IsValid(InteractCollider))
	{
		return nullptr;
	}

	return InteractCollider;
}

void ADC1InteractBuildActor::GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const
{
	OutOptions.Reset();

	switch (BuildState)
	{
	case EDC1BuildState::Construction :
		OutOptions = ConstructionOptions;
		break;
	case EDC1BuildState::Completed :
		OutOptions = CompletedOptions;
		break;
	default:
		return;
	}
}

void ADC1InteractBuildActor::Interact(ADC1Character* InInteractOwner, EDC1InteractKey InKey)
{
	if (false == ::IsValid(InInteractOwner))
	{
		return;
	}

	if (true == IsUnderConstruction() && InKey == EDC1InteractKey::C)
	{
		ADC1PlayerController* PC = InInteractOwner->GetController<ADC1PlayerController>();
		if (true == ::IsValid(PC))
		{
			UDC1InteractComponent* InteractComp = PC->GetInteractComponent();
			if (true == ::IsValid(InteractComp))
			{
				InteractComp->RequestCancelBuild(this);
			}
		}

		return;
	}

	if (BuildState == EDC1BuildState::Completed)
	{
		OnCompletedInteract(InInteractOwner, InKey);
	}
}

void ADC1InteractBuildActor::OnLocalFocusChanged(bool InIsFocused)
{
	IsLocalFocused = InIsFocused;
	UpdateInfoState();
}

void ADC1InteractBuildActor::InitBuild(const FDC1BuildActorData& InBuildActorData)
{
	BuildActorData = InBuildActorData;

	BuildState = InBuildActorData.RequiredWork > 0.f ? EDC1BuildState::Construction : EDC1BuildState::Completed;
}

void ADC1InteractBuildActor::BuildDestroy()
{
	CancelWorkersAbility();
	ClearWorkers();
	ClearPalWorkers();

	Super::BuildDestroy();
}

void ADC1InteractBuildActor::ApplyState()
{
	Super::ApplyState();

	if (BuildState == EDC1BuildState::Default || false == ::IsValid(InteractCollider))
	{
		return;
	}

	ECollisionEnabled::Type EnabledType = (BuildState == EDC1BuildState::Preview || BuildState == EDC1BuildState::Destroying) ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryOnly;

	InteractCollider->SetCollisionEnabled(EnabledType);

	if (BuildState == EDC1BuildState::Construction)
	{
		BuildMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CreateTranslucentMIDs();
		SetMaterialTranslucentColor(ConstructionColor);
		SetMaterialBuildProgress();
		SetActorTickEnabled(true);

		// DEBUG
		if (true == HasAuthority() && DebugAssignRadius > 0.f)
		{
			DebugAssignNearbyPals();
		}
	}
	else if (BuildState == EDC1BuildState::Completed)
	{
		// Construction을 거쳐온 경우에만 MID가 남아 있음 - Flash 후 원본 머티리얼로 복귀
		if (false == BuildMIDs.IsEmpty())
		{
			FlashAmount = 1.f;
			SetActorTickEnabled(true);
		}

		OnBuildCompleted.Broadcast();

		ClearWorkers();
		ClearPalWorkers();

		// DEBUG 생산 적성 팰 배정
		if (true == HasAuthority() && DebugAssignRadius > 0.f)
		{
			DebugAssignNearbyPals();
		}
	}

	OnInteractOptionsChanged.Broadcast();

	UpdateInfoState();
}

bool ADC1InteractBuildActor::CanAcceptWorker(const ADC1Character* InWorker) const
{
	const ADC1PalCharacter* Pal = Cast<const ADC1PalCharacter>(InWorker);
	if (true == ::IsValid(Pal))
	{
		return IsPalWorkerAssigned(Pal) && CanPalWork();
	}

	return IsUnderConstruction();
}

void ADC1InteractBuildActor::OnWorkTick(float InWork)
{
	if (BuildState != EDC1BuildState::Construction)
	{
		return;
	}


	if (BuildActorData.RequiredWork > 0.f && CurrentWork < BuildActorData.RequiredWork)
	{
		CurrentWork += InWork;
		
		BuildProgress = (CurrentWork / BuildActorData.RequiredWork);

		// Standalone 처리
		OnWorkProgressChanged.Broadcast();

		if (BuildProgress >= 1.f)
		{
			BuildProgress = 1.f;

			SetState(EDC1BuildState::Completed);
		}

		SetMaterialBuildProgress();
	}
}

EDC1BuildInfoState ADC1InteractBuildActor::ComputeInfoState() const
{
	if (BuildState == EDC1BuildState::Construction && true == IsLocalFocused)
	{
		return EDC1BuildInfoState::Construction;
	}

	return EDC1BuildInfoState::Hidden;
}

TSubclassOf<UDC1BuildInfoWidget> ADC1InteractBuildActor::GetInfoWidgetClass(EDC1BuildInfoState InState) const
{
	if (BuildState == EDC1BuildState::Construction)
	{
		return ConstructionInfoWidgetClass;
	}

	return nullptr;
}

bool ADC1InteractBuildActor::CanPalWork() const
{
	return IsUnderConstruction();
}

void ADC1InteractBuildActor::AddWorker(const ADC1Character* InWorker)
{
	if (false == ::IsValid(InWorker) || false == HasAuthority() || false == CanAcceptWorker(InWorker))
	{
		return;
	}

	Workers.Add(InWorker);

	SetActorTickEnabled(true);
}

void ADC1InteractBuildActor::RemoveWorker(const ADC1Character* InWorker)
{
	if (false == ::IsValid(InWorker) || false == HasAuthority())
	{
		return;
	}

	Workers.Remove(InWorker);
}

bool ADC1InteractBuildActor::AssignPalWorker(ADC1PalCharacter* InPal)
{
	if (false == HasAuthority() || false == ::IsValid(InPal))
	{
		return false;
	}

	bool IsChange = false;

	PalWorkers.RemoveAll([&IsChange](TWeakObjectPtr<ADC1PalCharacter> Worker) {
		if (false == Worker.IsValid())
		{
			IsChange = true;
			return true;
		}
		else
			return false;});

	if (IsPalWorkerAssigned(InPal))
	{
		if (true == IsChange)
		{
			ApplyPalWorkers();
		}
		return true;
	}

	if (false == CanPalWork() || false == HasPalWorkerSlot() || false == CanAssignPal(InPal))
	{
		if (true == IsChange)
		{
			ApplyPalWorkers();
		}

		return false;
	}

	PalWorkers.Add(InPal);
	ApplyPalWorkers();

	return true;
}

void ADC1InteractBuildActor::UnassignPalWorker(ADC1PalCharacter* InPal)
{
	if (false == HasAuthority())
	{
		return;
	}

	RemoveWorker(InPal);
	if (PalWorkers.Remove(InPal) > 0)
	{
		ApplyPalWorkers();
	}
}

void ADC1InteractBuildActor::ClearWorkers()
{
	Workers.Empty();

	SetCurrentWorkSpeed(0.f);
}

void ADC1InteractBuildActor::ClearPalWorkers()
{
	if (false == HasAuthority())
	{
		return;
	}

	if (true == PalWorkers.IsEmpty())
	{
		return;
	}

	PalWorkers.Empty();
	ApplyPalWorkers();
}

void ADC1InteractBuildActor::UpdateInfoState()
{
	EDC1BuildInfoState NewState = ComputeInfoState();
	if (InfoState == NewState)
	{
		return;
	}

	if (NewState != EDC1BuildInfoState::Hidden)
	{
		TSubclassOf<UDC1BuildInfoWidget> WidgetClass = GetInfoWidgetClass(NewState);
		if (false == ::IsValid(WidgetClass))
		{
			UE_LOG(LogTemp, Warning, TEXT("ADC1InteractBuildActor :: Empty Widget Class To [%s]"), *GetNameSafe(this));
			NewState = EDC1BuildInfoState::Hidden;
		}
		else if(InfoWidgetComponent->GetWidgetClass() != WidgetClass)
		{
			ReleaseInfoWidget();
			InfoWidgetComponent->SetWidgetClass(WidgetClass);
		}
	}

	EDC1BuildInfoState PrevState = InfoState;
	InfoState = NewState;
	InfoWidgetComponent->SetVisibility(InfoState != EDC1BuildInfoState::Hidden);

	UDC1BuildInfoWidget* Widget = AcquireInfoWidget();
	if (true == ::IsValid(Widget))
	{
		Widget->SetInfoState(PrevState, InfoState);
	}
}

void ADC1InteractBuildActor::OnRep_BuildProgress()
{
	SetMaterialBuildProgress();

	OnWorkProgressChanged.Broadcast();
}

void ADC1InteractBuildActor::OnRep_CurrentWorkSpeed()
{
	OnWorkProgressChanged.Broadcast();
}

void ADC1InteractBuildActor::OnRep_PalWorkers()
{
	ApplyPalWorkers();
}

void ADC1InteractBuildActor::BuildCompleted()
{
	if (true == ::IsValid(BuildMesh))
	{
		BuildMesh->EmptyOverrideMaterials();
	}

	BuildMIDs.Reset();
}

void ADC1InteractBuildActor::CancelWorkersAbility()
{
	FGameplayTagContainer WorkTags;
	WorkTags.AddTag(DC1StateTags::State_Working);

	// 서버 환경에서 RemoveWorker로 인한 동기화 문제 방지
	TSet<TWeakObjectPtr<const ADC1Character>> CachedWorker = Workers;

	for (TWeakObjectPtr<const ADC1Character> Worker : CachedWorker)
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

float ADC1InteractBuildActor::ComputeTotalWorkSpeed()
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

void ADC1InteractBuildActor::SetMaterialBuildProgress()
{
	int32 NumMIDs = BuildMIDs.Num();

	for (int32 i = 0; i < NumMIDs; ++i)
	{
		if (false == ::IsValid(BuildMIDs[i]))
		{
			continue;
		}

		BuildMIDs[i]->SetScalarParameterValue(DC1BuildMaterialParams::BuildProgress, BuildProgress);
	}
}

void ADC1InteractBuildActor::SetMaterialFlash()
{
	for (UMaterialInstanceDynamic* MID : BuildMIDs)
	{
		if (true == ::IsValid(MID))
		{
			MID->SetScalarParameterValue(DC1BuildMaterialParams::FlashAmount, FlashAmount);
		}
	}
}

void ADC1InteractBuildActor::SetCurrentWorkSpeed(float InWorkSpeed)
{
	if (false == HasAuthority())
	{
		return;
	}

	if (true == FMath::IsNearlyEqual(CurrentWorkSpeed, InWorkSpeed))
	{
		return;
	}

	CurrentWorkSpeed = InWorkSpeed;

	OnWorkProgressChanged.Broadcast();
}

UDC1BuildInfoWidget* ADC1InteractBuildActor::AcquireInfoWidget()
{
	UDC1BuildInfoWidget* Widget = Cast<UDC1BuildInfoWidget>(InfoWidgetComponent->GetUserWidgetObject());
	if (false == ::IsValid(Widget) && InfoState != EDC1BuildInfoState::Hidden)
	{
		InfoWidgetComponent->InitWidget();

		Widget = Cast<UDC1BuildInfoWidget>(InfoWidgetComponent->GetUserWidgetObject());
	}

	if (true == ::IsValid(Widget))
	{
		Widget->BindBuildActor(this);
	}

	return Widget;
}

void ADC1InteractBuildActor::ReleaseInfoWidget()
{
	UDC1BuildInfoWidget* Widget = Cast<UDC1BuildInfoWidget>(InfoWidgetComponent->GetUserWidgetObject());
	if (true == ::IsValid(Widget))
	{
		Widget->UnbindBuildActor();
	}
}

bool ADC1InteractBuildActor::CanAssignPal(const ADC1PalCharacter* InPal) const
{
	if (false == ::IsValid(InPal) || EDC1Team::Player != InPal->GetTeam())
	{
		return false;
	}

	FGameplayTag RequiredTag = GetRequiredWorkTag();
	if (false == RequiredTag.IsValid())
	{
		return false;
	}

	const UDC1PalData* PalData = InPal->GetPalData();
	if (false == ::IsValid(PalData) || false == PalData->WorkSuitability.Contains(RequiredTag))
	{
		return false;
	}

	// 거점 배치 팰
	ADC1PalBoxActor* PalBox = InPal->GetBasePalBox();
	if (true == ::IsValid(PalBox))
	{
		return PalBox->IsInBase(GetActorLocation());
	}

	// 소환된 팰 , TODO 거리 제한 
	return true;
}

void ADC1InteractBuildActor::ApplyPalWorkers()
{
	OnPalWorkersChanged.Broadcast();
}

void ADC1InteractBuildActor::DebugAssignNearbyPals()
{
	// TEMP v6 - 첫 성공 팰만 작업(AddWorker), 나머지는 배정만(휴식) + SAN 랜덤으로 복제 확인 ( v7에서 삭제 )
	const FVector Center = GetActorLocation();
	const float RadiusSq = FMath::Square(DebugAssignRadius);
	bool IsWorkerAdded = false;

	for (TActorIterator<ADC1PalCharacter> It(GetWorld()); It; ++It)
	{
		ADC1PalCharacter* Pal = *It;
		if (false == ::IsValid(Pal) || FVector::DistSquared(Center, Pal->GetActorLocation()) > RadiusSq)
		{
			continue;
		}

		if (false == AssignPalWorker(Pal))
		{
			UE_LOG(LogTemp, Warning, TEXT("ADC1InteractBuildActor :: DebugAssign Rejected - [%s] -> [%s]"), *GetNameSafe(Pal), *GetNameSafe(this));
			continue;
		}

		if (false == IsWorkerAdded)
		{
			AddWorker(Pal);
			IsWorkerAdded = true;
		}

		UAbilitySystemComponent* ASC = Pal->GetAbilitySystemComponent();
		if (true == ::IsValid(ASC))
		{
			ASC->SetNumericAttributeBase(UDC1PalAttributeSet::GetSanityAttribute(), FMath::FRandRange(30.f, 100.f));
		}
	}
}
