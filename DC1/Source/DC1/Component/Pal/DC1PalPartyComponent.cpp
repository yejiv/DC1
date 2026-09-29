// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Pal/DC1PalPartyComponent.h"

#include "Character/Pal/DC1PalCharacter.h"

#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

UDC1PalPartyComponent::UDC1PalPartyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UDC1PalPartyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDC1PalPartyComponent, Pals);
	DOREPLIFETIME(UDC1PalPartyComponent, DeployedPal);
	DOREPLIFETIME(UDC1PalPartyComponent, BoxPals);
}

void UDC1PalPartyComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	if (Pals.Num() < Capacity)
	{
		Pals.SetNum(Capacity);
	}

	if (BoxPals.Num() < BoxCapacity)
	{
		BoxPals.SetNum(BoxCapacity);
	}

	GetWorld()->GetTimerManager().SetTimer(BoxHealTimer, this, &ThisClass::HealBoxPals, BoxHealInterval, true);
}

void UDC1PalPartyComponent::OnRep_Pals()
{
	OnPalSlotChanged.Broadcast();
}

void UDC1PalPartyComponent::OnRep_DeployedPal()
{
	OnPalSlotChanged.Broadcast();
}

void UDC1PalPartyComponent::OnRep_BoxPals()
{
	OnPalBoxChanged.Broadcast();
}

bool UDC1PalPartyComponent::HasPalAt(int32 InSlotIndex) const
{
	return Pals.IsValidIndex(InSlotIndex) && Pals[InSlotIndex].IsValidPal();
}

bool UDC1PalPartyComponent::IsPalAlive(int32 InSlotIndex) const
{
	return Pals.IsValidIndex(InSlotIndex) && Pals[InSlotIndex].IsDeployable();
}

bool UDC1PalPartyComponent::IsDeployed(int32 InSlotIndex) const
{
	if (false == ::IsValid(DeployedPal) || false == Pals.IsValidIndex(InSlotIndex))
	{
		return false;
	}

	// 슬롯을 따로 들고 있지 않는다. 액터의 InstanceID 로 목록에서 찾는다.
	// 값이 둘이면 복제 도착 순서에 따라 어긋날 수 있다
	return (Pals[InSlotIndex].PalID == DeployedPal->GetInstanceID());
}

void UDC1PalPartyComponent::ExportSaveData(TArray<FDC1PalInstance>& Out) const
{
	Out = Pals;
}

void UDC1PalPartyComponent::ImportSaveData(const TArray<FDC1PalInstance>& In)
{
	// Server만 동작
	if (true == GetOwner()->HasAuthority())
	{
		Pals = In;
		OnPalSlotChanged.Broadcast();
	}
}

void UDC1PalPartyComponent::HandlePalCaptured(const FDC1PalInstance& InPal)
{
	if (ROLE_Authority != GetOwnerRole() || false == InPal.IsValidPal())
	{
		return;
	}

	// 포획 경험치 (잡힌 팰 제외)
	GrantExperienceFor(InPal.Level);

	// 파티 → 상자 순서로 빈칸
	for (const EDC1PalContainer Container : { EDC1PalContainer::Party, EDC1PalContainer::Box })
	{
		TArray<FDC1PalInstance>& Target = GetContainer(Container);

		const int32 EmptyIndex = Target.IndexOfByPredicate([](const FDC1PalInstance& Pal)
			{
				return (false == Pal.IsValidPal());
			});

		if (INDEX_NONE == EmptyIndex)
		{
			continue;
		}

		Target[EmptyIndex] = InPal;

		BroadcastChanged(Container);
		return;
	}
}

void UDC1PalPartyComponent::Server_MovePal_Implementation(EDC1PalContainer InFrom, int32 InFromIndex, EDC1PalContainer InTo, int32 InToIndex)
{
	if (false == GetContainer(InFrom).IsValidIndex(InFromIndex) || false == GetContainer(InTo).IsValidIndex(InToIndex))
	{
		return;
	}

	if (InFrom == InTo && InFromIndex == InToIndex)
	{
		return;
	}

	if (false == GetContainer(InFrom)[InFromIndex].IsValidPal())
	{
		return;
	}

	// 소환 중인 팰이 상자로 가면 회수
	const bool bDeployedToBox = (EDC1PalContainer::Party == InFrom && EDC1PalContainer::Box == InTo && true == IsDeployed(InFromIndex))
		|| (EDC1PalContainer::Box == InFrom && EDC1PalContainer::Party == InTo && true == IsDeployed(InToIndex));

	if (true == bDeployedToBox)
	{
		RetrieveDeployed();
	}

	Swap(GetContainer(InFrom)[InFromIndex], GetContainer(InTo)[InToIndex]);

	BroadcastChanged(InFrom);
	if (InFrom != InTo)
	{
		BroadcastChanged(InTo);
	}
}

bool UDC1PalPartyComponent::CanAcceptPal() const
{
	auto HasEmpty = [](const TArray<FDC1PalInstance>& InPals)
		{
			return InPals.ContainsByPredicate([](const FDC1PalInstance& Pal) { return (false == Pal.IsValidPal()); });
		};

	return HasEmpty(Pals) || HasEmpty(BoxPals);
}

void UDC1PalPartyComponent::HealBoxPals()
{
	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == ::IsValid(Registry) || Registry->BoxHealRatePerSecond <= 0.f)
	{
		return;
	}

	bool bChanged = false;

	for (FDC1PalInstance& Pal : BoxPals)
	{
		if (false == Pal.IsValidPal())
		{
			continue;
		}

		const float MaxHealth = GetMaxHealthOf(Pal);
		if (Pal.CurrentHealth >= MaxHealth)
		{
			continue;
		}

		Pal.CurrentHealth = FMath::Min(MaxHealth, Pal.CurrentHealth + (MaxHealth * Registry->BoxHealRatePerSecond * BoxHealInterval));
		bChanged = true;
	}

	if (true == bChanged)
	{
		BroadcastChanged(EDC1PalContainer::Box);
	}
}

TArray<FDC1PalInstance>& UDC1PalPartyComponent::GetContainer(EDC1PalContainer InContainer)
{
	return (EDC1PalContainer::Box == InContainer) ? BoxPals : Pals;
}

const TArray<FDC1PalInstance>& UDC1PalPartyComponent::GetContainer(EDC1PalContainer InContainer) const
{
	return (EDC1PalContainer::Box == InContainer) ? BoxPals : Pals;
}

void UDC1PalPartyComponent::BroadcastChanged(EDC1PalContainer InContainer)
{
	if (EDC1PalContainer::Box == InContainer)
	{
		OnPalBoxChanged.Broadcast();
	}
	else
	{
		OnPalSlotChanged.Broadcast();
	}
}

const FDC1PalInstance* UDC1PalPartyComponent::GetPalInfo(EDC1PalContainer InContainer, int32 InSlotIndex) const
{
	if (EDC1PalContainer::Party == InContainer)
	{
		return GetPalInfo(InSlotIndex);
	}

	return (BoxPals.IsValidIndex(InSlotIndex) && BoxPals[InSlotIndex].IsValidPal()) ? &BoxPals[InSlotIndex] : nullptr;
}

float UDC1PalPartyComponent::GetPalHealth(EDC1PalContainer InContainer, int32 InSlotIndex) const
{
	if (EDC1PalContainer::Party == InContainer)
	{
		return GetPalHealth(InSlotIndex);
	}

	const FDC1PalInstance* Pal = GetPalInfo(InContainer, InSlotIndex);
	return (nullptr != Pal) ? Pal->CurrentHealth : 0.f;
}

float UDC1PalPartyComponent::GetPalMaxHealth(EDC1PalContainer InContainer, int32 InSlotIndex) const
{
	const FDC1PalInstance* Pal = GetPalInfo(InContainer, InSlotIndex);
	return (nullptr != Pal) ? GetMaxHealthOf(*Pal) : 0.f;
}

float UDC1PalPartyComponent::GetMaxHealthOf(const FDC1PalInstance& InPal) const
{
	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == ::IsValid(Registry))
	{
		return 0.f;
	}

	const UDC1PalData* PalData = Registry->FindPalDataByTag(InPal.SpeciesTag).LoadSynchronous();
	if (false == ::IsValid(PalData))
	{
		return 0.f;
	}

	return Registry->GetMaxHealthFor(*PalData, InPal.Level, InPal.TalentHealth);
}

void UDC1PalPartyComponent::GrantExperienceFor(int32 InDefeatedLevel)
{
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == ::IsValid(Registry))
	{
		return;
	}

	const int32 Amount = Registry->GetDropExp(InDefeatedLevel);
	if (Amount <= 0)
	{
		return;
	}

	for (int32 Index = 0; Index < Pals.Num(); ++Index)
	{
		if (false == HasPalAt(Index))
		{
			continue;
		}

		// 소환 중이면 액터 기준
		if (true == IsDeployed(Index))
		{
			DeployedPal->GainExperience(Amount);
			Pals[Index].Level = DeployedPal->GetLevel();
			Pals[Index].Experience = DeployedPal->GetExperience();
		}
		else
		{
			Registry->ApplyExperience(Pals[Index], Amount);
		}
	}

	OnPalSlotChanged.Broadcast();
}

void UDC1PalPartyComponent::Server_ToggleDeploy_Implementation()
{
	ToggleDeploy();
}

void UDC1PalPartyComponent::ToggleDeploy()
{
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	// 꺼내둔 게 있으면 접는다
	if (::IsValid(DeployedPal))
	{
		RetrieveDeployed();
		return;
	}

	if (false == HasPalAt(SelectedSlot))
	{
		return;
	}

	if (false == Pals[SelectedSlot].IsDeployable())
	{
		return;
	}

	FTransform SpawnTransform;
	if (false == FindDeployTransform(SpawnTransform))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Deploy: no navigable spot near the owner."));
		return;
	}

	ADC1PalCharacter* Spawned = ADC1PalCharacter::SpawnFromInstanceData(GetWorld(), Pals[SelectedSlot], SpawnTransform);
	if (false == ::IsValid(Spawned))
	{
		return;
	}

	Spawned->OnPalDied.AddUObject(this, &ThisClass::HandleDeployedPalDied);
	DeployedPal = Spawned;

	OnPalSlotChanged.Broadcast();
}

void UDC1PalPartyComponent::CycleSlot(int32 InDelta)
{
	if (0 == InDelta || Capacity <= 0)
	{
		return;
	}

	SelectedSlot = (SelectedSlot + InDelta + Capacity) % Capacity;

	Server_SetSelectedSlot(SelectedSlot);
	OnPalSlotTranslated.Broadcast();
}

const FDC1PalInstance* UDC1PalPartyComponent::GetPalInfo(int32 Index) const
{
	if (false == Pals.IsValidIndex(Index))
		return nullptr;

	if (false == HasPalAt(Index))
		return nullptr;

	return &Pals[Index];
}

float UDC1PalPartyComponent::GetPalMaxHealth(int32 InSlotIndex) const
{
	return HasPalAt(InSlotIndex) ? GetMaxHealthOf(Pals[InSlotIndex]) : 0.f;
}

float UDC1PalPartyComponent::GetPalHealth(int32 InSlotIndex) const
{
	if (false == HasPalAt(InSlotIndex))
	{
		return 0.f;
	}

	if (true == IsDeployed(InSlotIndex))
	{
		return DeployedPal->GetHealthRatio() * GetPalMaxHealth(InSlotIndex);
	}

	return Pals[InSlotIndex].CurrentHealth;
}

void UDC1PalPartyComponent::Server_SetSelectedSlot_Implementation(int32 InSlotIndex)
{
	SelectedSlot = FMath::Clamp(InSlotIndex, 0, Capacity - 1);
}

void UDC1PalPartyComponent::RetrieveDeployed()
{
	GetWorld()->GetTimerManager().ClearTimer(DeathRetrieveTimer);

	ADC1PalCharacter* Deployed = DeployedPal;
	DeployedPal = nullptr;

	if (false == ::IsValid(Deployed))
	{
		return;
	}

	UpdatePal(Deployed->Fold());
}

void UDC1PalPartyComponent::HandleDeployedPalDied()
{
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	// UI 는 죽는 즉시 반응
	OnPalSlotChanged.Broadcast();

	GetWorld()->GetTimerManager().SetTimer(DeathRetrieveTimer, this, &ThisClass::RetrieveDeployed, DeathRetrieveDelay, false);
}

void UDC1PalPartyComponent::UpdatePal(const FDC1PalInstance& InPal)
{
	const int32 Index = Pals.IndexOfByPredicate([&InPal](const FDC1PalInstance& Pal)
		{
			return (Pal.PalID == InPal.PalID);
		});

	if (INDEX_NONE == Index)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] UpdatePal: no entry for ID %s."), *InPal.PalID.ToString(EGuidFormats::Digits));
		return;
	}

	Pals[Index] = InPal;
	OnPalSlotChanged.Broadcast();
}

bool UDC1PalPartyComponent::FindDeployTransform(FTransform& OutTransform) const
{
	const APlayerState* OwnerState = Cast<APlayerState>(GetOwner());
	const APawn* OwnerPawn = (nullptr != OwnerState) ? OwnerState->GetPawn() : nullptr;
	if (false == ::IsValid(OwnerPawn))
	{
		return false;
	}

	const FVector Ideal = OwnerPawn->GetActorLocation() + (OwnerPawn->GetActorForwardVector() * DeployDistance);

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (nullptr == NavSystem)
	{
		return false;
	}

	FNavLocation Projected;
	if (false == NavSystem->ProjectPointToNavigation(Ideal, Projected, FVector(DeployDistance)))
	{
		return false;
	}

	OutTransform = FTransform(Projected.Location + FVector(0.f, 0.f, OwnerPawn->GetSimpleCollisionHalfHeight()));

	return true;
}
