// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Interact/Production/DC1ProductionBuildActor.h"

#include "Net/UnrealNetwork.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Character/Player/DC1PlayerController.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Interact/DC1InteractComponent.h"
#include "Component/Inventory/Player/DC1PlayerInventoryComponent.h"

#include "Components/SphereComponent.h"

#include "Define/DC1Enums.h"

//DEBUG
#include "EngineUtils.h"

ADC1ProductionBuildActor::ADC1ProductionBuildActor()
{
	// AbilityTag 채우기 - 제작, 채광, 벌목
	CompletedOptions.Emplace(EDC1InteractKey::F, NSLOCTEXT("Interact", "Production", "생산"), EDC1InteractInputType::HoldAbility, FGameplayTag::EmptyTag);

	// AbilityTag 비워두기
	CollectOptions.Emplace(EDC1InteractKey::F, NSLOCTEXT("Interact", "Collect", "획득"), EDC1InteractInputType::Press, FGameplayTag::EmptyTag);

	InfoRangeSphere = CreateDefaultSubobject<USphereComponent>("InfoRangeSphere");
	InfoRangeSphere->SetupAttachment(BuildMesh);
	InfoRangeSphere->SetCollisionProfileName(DC1CollisionProfile::BuildInteractionInfo);
	InfoRangeSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InfoRangeSphere->SetGenerateOverlapEvents(true);
	InfoRangeSphere->SetCanEverAffectNavigation(false);
	
}

FGameplayTag ADC1ProductionBuildActor::GetRequiredWorkTag() const
{
	return (true == IsCompleted()) ? RequiredWorkTag : Super::GetRequiredWorkTag();
}

void ADC1ProductionBuildActor::BeginPlay()
{
	Super::BeginPlay();

	if (true == ::IsValid(InfoRangeSphere))
	{
		InfoRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnInfoRangeBeginOverlap);
		InfoRangeSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnInfoRangeEndOverlap);
	}

	RefreshLocalInRange();
	UpdateInfoState();
}

void ADC1ProductionBuildActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ProductionProgress);
	DOREPLIFETIME(ThisClass, StoredCount);
	DOREPLIFETIME(ThisClass, PalWorkers);
}

void ADC1ProductionBuildActor::GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const
{
	Super::GetInteractOptions(OutOptions);

	if (false == IsCompleted() || StoredCount <= 0)
	{
		return;
	}

	for (const FDC1InteractOption& CollectOption : CollectOptions)
	{
		FDC1InteractOption* ChangeOption = OutOptions.FindByPredicate([&CollectOption](const FDC1InteractOption& Option)
			{
				return Option.Key == CollectOption.Key;
			});
		if (nullptr != ChangeOption)
		{
			*ChangeOption = CollectOption;
		}
		else
		{
			OutOptions.Add(CollectOption);
		}
	}
}

bool ADC1ProductionBuildActor::ShouldKeepOptionsWhileHeld() const
{
	return (true == IsCompleted()) && (StoredCount < MaxStoredCount);
}

void ADC1ProductionBuildActor::CollectProduct(ADC1Character* InCollect)
{
	if (false == HasAuthority() || false == ::IsValid(InCollect) ||
		(false == IsCompleted()) || StoredCount <= 0 || (true == ProductItemID.IsNone()))
	{
		return;
	}

	ADC1PlayerState* PlayerState = InCollect->GetPlayerState<ADC1PlayerState>();
	if (false == ::IsValid(PlayerState))
	{
		return;
	}

	UDC1PlayerInventoryComponent* InventoryComp = PlayerState->GetInventoryComponent();
	if (false == ::IsValid(InventoryComp))
	{
		return;
	}

	if (true == InventoryComp->AddItem(ProductItemID, StoredCount))
	{
		SetStoredCount(0);
	}
	else
	{
		// TODO - Drop Item
		// TODO - SetStoredCount(0);
	}
}

void ADC1ProductionBuildActor::ApplyState()
{
	bool IsLocalAndIsCompleted = (true == IsCompleted()) && (NM_DedicatedServer != GetNetMode());
	ECollisionEnabled::Type CollisionType = IsLocalAndIsCompleted ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision;
	
	InfoRangeSphere->SetCollisionEnabled(CollisionType);
	
	RefreshLocalInRange();

	Super::ApplyState();
}

bool ADC1ProductionBuildActor::CanAcceptWorker(const ADC1Character* InWorker) const
{
	if (true == Super::CanAcceptWorker(InWorker))
	{
		return true;
	}

	const ADC1PalCharacter* Pal = Cast<const ADC1PalCharacter>(InWorker);
	if (nullptr != Pal)
	{
		// 팰은 Super에서 전부 판단
		return false;
	}

	return (true == CanProduce()) && (StoredCount <= 0);
}

void ADC1ProductionBuildActor::OnWorkTick(float InWork)
{
	if (false == IsCompleted())
	{
		Super::OnWorkTick(InWork);
		return;
	}

	if (RequiredWorkPerItem <= 0.f || StoredCount >= MaxStoredCount)
	{
		return;
	}

	CurrentProductionWork += InWork;

	int32 NewCount = StoredCount;

	while (CurrentProductionWork >= RequiredWorkPerItem && NewCount < MaxStoredCount)
	{
		CurrentProductionWork -= RequiredWorkPerItem;
		++NewCount;
	}

	if (NewCount >= MaxStoredCount)
	{
		CurrentProductionWork = 0.f;
		ClearWorkers();
		ClearPalWokrers();
	}

	ProductionProgress = CurrentProductionWork / RequiredWorkPerItem;

	if (NewCount <= StoredCount)
	{
		OnProductionChanged.Broadcast();
		return;
	}


	SetStoredCount(NewCount);
}

EDC1BuildInfoState ADC1ProductionBuildActor::ComputeInfoState() const
{
	if (false == IsCompleted())
	{
		return Super::ComputeInfoState();
	}

	if (true == ProductItemID.IsNone() && (IsLocalFocused || IsLocalInRange))
	{
		return EDC1BuildInfoState::ProductionNoItem;
	}

	if (true == IsLocalFocused)
	{
		return EDC1BuildInfoState::ProductionDetail;
	}

	if (true == IsLocalInRange)
	{
		return EDC1BuildInfoState::ProductionCompact;
	}

	return EDC1BuildInfoState::Hidden;
}

TSubclassOf<UDC1BuildInfoWidget> ADC1ProductionBuildActor::GetInfoWidgetClass(EDC1BuildInfoState InState) const
{
	if (true == IsCompleted())
	{
		return ProductionInfoWidgetClass;
	}

	return Super::GetInfoWidgetClass(InState);
}

void ADC1ProductionBuildActor::OnCompletedInteract(ADC1Character* InInteractOwner, EDC1InteractKey InKey)
{
	if (StoredCount <= 0)
	{
		return;
	}

	if (false == CollectOptions.ContainsByPredicate([&InKey](const FDC1InteractOption& Option) { return Option.Key == InKey; }))
	{
		return;
	}

	ADC1PlayerController* PC = InInteractOwner->GetController<ADC1PlayerController>();
	if (true == ::IsValid(PC))
	{
		UDC1InteractComponent* InteractComp = PC->GetInteractComponent();
		if (true == ::IsValid(InteractComp))
		{
			InteractComp->RequestCollectProduct(this);
		}
	}
}

bool ADC1ProductionBuildActor::CanPalWork() const
{
	return (Super::CanPalWork() || ( CanProduce() && StoredCount < MaxStoredCount ));
}

bool ADC1ProductionBuildActor::HasPalWorkerSlot() const
{
	return (true == IsCompleted()) ? PalWorkers.Num() < MaxPalWorekrs : Super::HasPalWorkerSlot();
}

void ADC1ProductionBuildActor::OnRep_ProductionProgress()
{
	OnProductionChanged.Broadcast();
}

void ADC1ProductionBuildActor::OnRep_StoredCount(int32 OldCount)
{
	ApplyStoredCount(OldCount);
}

APawn* ADC1ProductionBuildActor::GetLocalPawn() const
{
	UGameInstance* GameInstance = GetGameInstance();
	if (true == ::IsValid(GameInstance))
	{
		APlayerController* PC = GameInstance->GetFirstLocalPlayerController();
		if (true == ::IsValid(PC))
		{
			return PC->GetPawn();
		}
	}

	return nullptr;
}

void ADC1ProductionBuildActor::SetStoredCount(int32 InNewCount)
{
	if (false == HasAuthority())
	{
		return;
	}

	int32 OldCount = StoredCount;

	StoredCount = FMath::Clamp(InNewCount, 0, MaxStoredCount);

	if (OldCount != StoredCount)
	{
		// Standalone 표시용
		ApplyStoredCount(OldCount);
	}
}

void ADC1ProductionBuildActor::ApplyStoredCount(int32 InOldCount)
{
	bool IsEmptyChanged = (InOldCount <= 0) != (StoredCount <= 0);
	bool IsReachedMax = (InOldCount < MaxStoredCount) && (StoredCount >= MaxStoredCount);

	if (IsEmptyChanged || IsReachedMax)
	{
		OnInteractOptionsChanged.Broadcast();
	}

	OnProductionChanged.Broadcast();
}

void ADC1ProductionBuildActor::RefreshLocalInRange()
{
	APawn* LocalPawn = GetLocalPawn();

	IsLocalInRange = (::IsValid(LocalPawn)) && InfoRangeSphere->IsOverlappingActor(LocalPawn);
}

bool ADC1ProductionBuildActor::CanProduce() const
{
	return (true == IsCompleted()) && (false == ProductItemID.IsNone()) && (RequiredWorkPerItem > 0.f);
}

void ADC1ProductionBuildActor::OnInfoRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != GetLocalPawn())
	{
		return;
	}

	RefreshLocalInRange();
	UpdateInfoState();
}

void ADC1ProductionBuildActor::OnInfoRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != GetLocalPawn())
	{
		return;
	}

	RefreshLocalInRange();
	UpdateInfoState();
}
