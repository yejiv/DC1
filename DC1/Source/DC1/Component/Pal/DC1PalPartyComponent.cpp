// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Pal/DC1PalPartyComponent.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Controller/DC1PalAIController.h"
#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"

#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

namespace DC1PalParty
{
	// Possess 가 바꾼 시점을 되돌린다
	static void PossessKeepingView(APlayerController& InPC, APawn& InPawn)
	{
		const FRotator View = InPC.GetControlRotation();

		InPC.Possess(&InPawn);
		InPC.SetControlRotation(View);
		InPC.ClientSetRotation(View);
	}
}

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
	DOREPLIFETIME(UDC1PalPartyComponent, BasePals);
	DOREPLIFETIME(UDC1PalPartyComponent, BasePalBox);
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

	if (BasePals.Num() < BaseCapacity)
	{
		BasePals.SetNum(BaseCapacity);
	}

	GetWorld()->GetTimerManager().SetTimer(BoxHealTimer, this, &ThisClass::HealStoredPals, BoxHealInterval, true);
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

void UDC1PalPartyComponent::OnRep_BasePals()
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

	// 거점이 없으면 거점 칸에 못 넣는다
	if ((EDC1PalContainer::Base == InFrom || EDC1PalContainer::Base == InTo) && false == HasBase())
	{
		return;
	}

	// 소환 중인 팰이 파티를 떠나면 회수
	const bool bDeployedLeavesParty = (EDC1PalContainer::Party == InFrom && EDC1PalContainer::Party != InTo && true == IsDeployed(InFromIndex))
		|| (EDC1PalContainer::Party != InFrom && EDC1PalContainer::Party == InTo && true == IsDeployed(InToIndex));

	if (true == bDeployedLeavesParty)
	{
		RetrieveDeployed();
	}

	Swap(GetContainer(InFrom)[InFromIndex], GetContainer(InTo)[InToIndex]);

	if (EDC1PalContainer::Base == InFrom || EDC1PalContainer::Base == InTo)
	{
		RefreshBasePals();
	}

	BroadcastChanged(InFrom);
	if (InFrom != InTo)
	{
		BroadcastChanged(InTo);
	}
}

bool UDC1PalPartyComponent::HasBase() const
{
	return ::IsValid(BasePalBox);
}

void UDC1PalPartyComponent::Server_SetBase_Implementation(ADC1PalBoxActor* InPalBox)
{
	if (true == HasBase() || false == ::IsValid(InPalBox))
	{
		return;
	}

	BasePalBox = InPalBox;
	BasePalBox->OnEndPlay.AddDynamic(this, &ThisClass::HandleBaseEndPlay);

	RefreshBasePals();
	BroadcastChanged(EDC1PalContainer::Base);
}

void UDC1PalPartyComponent::RefreshBasePals()
{
	// 칸에서 빠졌거나 거점이 없어진 팰은 접는다. 체력은 옮겨간 칸에 반영된다
	for (int32 Index = BaseSpawnedPals.Num() - 1; Index >= 0; --Index)
	{
		ADC1PalCharacter* Spawned = BaseSpawnedPals[Index];
		if (false == ::IsValid(Spawned))
		{
			BaseSpawnedPals.RemoveAt(Index);
			continue;
		}

		const bool bStillInBase = HasBase() && BasePals.ContainsByPredicate([Spawned](const FDC1PalInstance& Pal)
			{
				return (Pal.PalID == Spawned->GetInstanceID());
			});

		if (false == bStillInBase)
		{
			BaseSpawnedPals.RemoveAt(Index);
			UpdatePal(Spawned->Fold());
		}
	}

	if (false == HasBase())
	{
		return;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (nullptr == NavSystem)
	{
		return;
	}

	// 칸에 있는데 안 나온 팰을 꺼낸다. 기절한 팰은 회복될 때까지 안 나온다
	for (const FDC1PalInstance& Pal : BasePals)
	{
		if (false == Pal.IsDeployable())
		{
			continue;
		}

		const bool bSpawned = BaseSpawnedPals.ContainsByPredicate([&Pal](const ADC1PalCharacter* Spawned)
			{
				return (Spawned->GetInstanceID() == Pal.PalID);
			});

		if (true == bSpawned)
		{
			continue;
		}

		FNavLocation SpawnLocation;
		if (false == NavSystem->GetRandomPointInNavigableRadius(BasePalBox->GetActorLocation(), BasePalBox->GetBaseRadius(), SpawnLocation))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Pal] Base: no navigable spot near the pal box."));
			return;
		}

		if (ADC1PalCharacter* Spawned = ADC1PalCharacter::SpawnFromInstanceData(GetWorld(), Pal, FTransform(SpawnLocation.Location), BasePalBox))
		{
			BaseSpawnedPals.Add(Spawned);
		}
	}
}

void UDC1PalPartyComponent::HandleBaseEndPlay(AActor* InActor, EEndPlayReason::Type InEndPlayReason)
{
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	BasePalBox = nullptr;

	// 월드의 거점 팰을 접어 칸에 반영한 뒤 옮긴다
	RefreshBasePals();

	for (FDC1PalInstance& Pal : BasePals)
	{
		if (false == Pal.IsValidPal())
		{
			continue;
		}

		bool bMoved = false;

		// 상자 → 파티 순서로 빈칸
		for (const EDC1PalContainer Container : { EDC1PalContainer::Box, EDC1PalContainer::Party })
		{
			TArray<FDC1PalInstance>& Target = GetContainer(Container);

			const int32 EmptyIndex = Target.IndexOfByPredicate([](const FDC1PalInstance& Other)
				{
					return (false == Other.IsValidPal());
				});

			if (INDEX_NONE != EmptyIndex)
			{
				Target[EmptyIndex] = Pal;
				bMoved = true;
				break;
			}
		}

		if (false == bMoved)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Pal] Base removed: box and party are full. Base pal %s was dropped."), *Pal.PalID.ToString(EGuidFormats::Digits));
		}

		Pal = FDC1PalInstance();
	}

	BroadcastChanged(EDC1PalContainer::Party);
	BroadcastChanged(EDC1PalContainer::Base);
}

bool UDC1PalPartyComponent::CanAcceptPal() const
{
	auto HasEmpty = [](const TArray<FDC1PalInstance>& InPals)
		{
			return InPals.ContainsByPredicate([](const FDC1PalInstance& Pal) { return (false == Pal.IsValidPal()); });
		};

	return HasEmpty(Pals) || HasEmpty(BoxPals);
}

void UDC1PalPartyComponent::HealStoredPals()
{
	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == ::IsValid(Registry) || Registry->BoxHealRatePerSecond <= 0.f)
	{
		return;
	}

	bool bChanged = false;

	for (const EDC1PalContainer Container : { EDC1PalContainer::Box, EDC1PalContainer::Base })
	{
		for (FDC1PalInstance& Pal : GetContainer(Container))
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
	}

	if (false == bChanged)
	{
		return;
	}

	// 나와 있는 거점 팰은 칸 값을 따른다. 거점 팰은 싸우지 않아 액터 체력이 따로 변하지 않는다
	for (ADC1PalCharacter* Spawned : BaseSpawnedPals)
	{
		const FDC1PalInstance* Pal = BasePals.FindByPredicate([Spawned](const FDC1PalInstance& Other)
			{
				return (::IsValid(Spawned) && Other.PalID == Spawned->GetInstanceID());
			});

		if (nullptr == Pal)
		{
			continue;
		}

		if (UAbilitySystemComponent* ASC = Spawned->GetAbilitySystemComponent())
		{
			ASC->SetNumericAttributeBase(UDC1AttributeSet::GetHealthAttribute(), Pal->CurrentHealth);
		}
	}

	// 기절에서 회복된 거점 팰을 꺼낸다
	RefreshBasePals();

	BroadcastChanged(EDC1PalContainer::Box);
}

TArray<FDC1PalInstance>& UDC1PalPartyComponent::GetContainer(EDC1PalContainer InContainer)
{
	switch (InContainer)
	{
	case EDC1PalContainer::Box:		return BoxPals;
	case EDC1PalContainer::Base:	return BasePals;
	default:						return Pals;
	}
}

const TArray<FDC1PalInstance>& UDC1PalPartyComponent::GetContainer(EDC1PalContainer InContainer) const
{
	return const_cast<UDC1PalPartyComponent*>(this)->GetContainer(InContainer);
}

void UDC1PalPartyComponent::BroadcastChanged(EDC1PalContainer InContainer)
{
	if (EDC1PalContainer::Party == InContainer)
	{
		OnPalSlotChanged.Broadcast();
	}
	else
	{
		OnPalBoxChanged.Broadcast();
	}
}

const FDC1PalInstance* UDC1PalPartyComponent::GetPalInfo(EDC1PalContainer InContainer, int32 InSlotIndex) const
{
	if (EDC1PalContainer::Party == InContainer)
	{
		return GetPalInfo(InSlotIndex);
	}

	const TArray<FDC1PalInstance>& Container = GetContainer(InContainer);
	return (Container.IsValidIndex(InSlotIndex) && Container[InSlotIndex].IsValidPal()) ? &Container[InSlotIndex] : nullptr;
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

	// 접기 전에 하차
	if (ADC1PalAIController* PalController = GetDeployedPalController())
	{
		PalController->CancelRideApproach();
	}
	EndRide(false);

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

	// 래그돌에서 하차
	if (ADC1PalAIController* PalController = GetDeployedPalController())
	{
		PalController->CancelRideApproach();
	}
	EndRide(false);

	// UI 는 죽는 즉시 반응
	OnPalSlotChanged.Broadcast();

	GetWorld()->GetTimerManager().SetTimer(DeathRetrieveTimer, this, &ThisClass::RetrieveDeployed, DeathRetrieveDelay, false);
}

void UDC1PalPartyComponent::Server_ToggleRide_Implementation()
{
	if (::IsValid(Rider))
	{
		EndRide(true);
		return;
	}

	const ADC1PalAIController* PalController = GetDeployedPalController();
	if (nullptr != PalController && true == PalController->IsRideApproaching())
	{
		return;
	}

	BeginRideApproach();
}

void UDC1PalPartyComponent::BeginRideApproach()
{
	ADC1PalCharacter* Pal = DeployedPal;
	const UDC1PalData* PalData = ::IsValid(Pal) ? Pal->GetPalData() : nullptr;
	if (nullptr == PalData || false == PalData->bRideable || Pal->GetHealthRatio() <= 0.f)
	{
		return;
	}

	if (false == Pal->GetMesh()->DoesSocketExist(PalData->RideSocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Ride: %s has no socket [%s]. Add it to the skeleton."),
			*GetNameSafe(Pal), *PalData->RideSocketName.ToString());
		return;
	}

	ADC1PalAIController* PalController = GetDeployedPalController();
	const APlayerController* PC = GetOwnerController();
	APawn* Player = (nullptr != PC) ? PC->GetPawn() : nullptr;
	if (nullptr == PalController || false == ::IsValid(Player))
	{
		return;
	}

	PalController->RequestRideApproach(Player, RideAcceptanceRadius,
		FDC1OnRideApproachFinished::CreateUObject(this, &ThisClass::HandleRideApproachFinished));
}

void UDC1PalPartyComponent::HandleRideApproachFinished(bool InArrived)
{
	// 실패 시 컨트롤러가 트리 복구
	if (true == InArrived)
	{
		Ride();
	}
}

void UDC1PalPartyComponent::Ride()
{
	ADC1PalCharacter* Pal = DeployedPal;
	const UDC1PalData* PalData = ::IsValid(Pal) ? Pal->GetPalData() : nullptr;
	APlayerController* PC = GetOwnerController();
	ACharacter* Player = (nullptr != PC) ? Cast<ACharacter>(PC->GetPawn()) : nullptr;
	if (nullptr == PalData || false == ::IsValid(Player))
	{
		return;
	}

	// 내릴 때 다시 Possess
	RiddenPalController = Pal->GetController();

	Pal->SetActorRotation(FRotator(0.f, PC->GetControlRotation().Yaw, 0.f));

	if (UCharacterMovementComponent* Movement = Player->GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	// 콜리전 유지 (탑승 중 피격)

	// 붙인 뒤 Possess (순서 중요: GetRiddenPawn)
	Player->AttachToComponent(Pal->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, PalData->RideSocketName);
	Rider = Player;

	DC1PalParty::PossessKeepingView(*PC, *Pal);
}

void UDC1PalPartyComponent::EndRide(bool InReturnPalToAI)
{
	ACharacter* Player = Rider;
	AController* PalController = RiddenPalController;
	Rider = nullptr;
	RiddenPalController = nullptr;

	if (false == ::IsValid(Player))
	{
		return;
	}

	// Possess 뒤 떼기 (순서 중요: GetRiddenPawn)
	if (APlayerController* PC = GetOwnerController())
	{
		DC1PalParty::PossessKeepingView(*PC, *Player);
	}

	// 못 찾으면 제자리
	FTransform DismountTransform;
	const bool HasSpot = FindDeployTransform(DismountTransform);

	Player->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (UCharacterMovementComponent* Movement = Player->GetCharacterMovement())
	{
		Movement->SetDefaultMovementMode();
	}

	const FRotator Upright(0.f, Player->GetActorRotation().Yaw, 0.f);
	if (true == HasSpot)
	{
		Player->TeleportTo(DismountTransform.GetLocation(), Upright);
	}
	else
	{
		Player->SetActorRotation(Upright);
	}

	if (false == ::IsValid(PalController))
	{
		return;
	}

	// 비켜 둔 AI 는 엔진이 정리 안 함
	ADC1PalCharacter* Pal = DeployedPal;
	if (true == InReturnPalToAI && ::IsValid(Pal))
	{
		PalController->Possess(Pal);
	}
	else
	{
		PalController->Destroy();
	}
}

APlayerController* UDC1PalPartyComponent::GetOwnerController() const
{
	const APlayerState* OwnerState = Cast<APlayerState>(GetOwner());
	return (nullptr != OwnerState) ? OwnerState->GetPlayerController() : nullptr;
}

ADC1PalAIController* UDC1PalPartyComponent::GetDeployedPalController() const
{
	return ::IsValid(DeployedPal) ? Cast<ADC1PalAIController>(DeployedPal->GetController()) : nullptr;
}

void UDC1PalPartyComponent::UpdatePal(const FDC1PalInstance& InPal)
{
	for (const EDC1PalContainer Container : { EDC1PalContainer::Party, EDC1PalContainer::Box, EDC1PalContainer::Base })
	{
		TArray<FDC1PalInstance>& Target = GetContainer(Container);

		const int32 Index = Target.IndexOfByPredicate([&InPal](const FDC1PalInstance& Pal)
			{
				return (Pal.PalID == InPal.PalID);
			});

		if (INDEX_NONE != Index)
		{
			Target[Index] = InPal;
			BroadcastChanged(Container);
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[Pal] UpdatePal: no entry for ID %s."), *InPal.PalID.ToString(EGuidFormats::Digits));
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
