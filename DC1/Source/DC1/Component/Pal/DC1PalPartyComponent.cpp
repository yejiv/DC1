// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Pal/DC1PalPartyComponent.h"

#include "Character/Pal/DC1PalCharacter.h"

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
}

void UDC1PalPartyComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ROLE_Authority == GetOwnerRole() && Pals.Num() < Capacity)
	{
		Pals.SetNum(Capacity);
	}
}

void UDC1PalPartyComponent::OnRep_Pals()
{
	OnPalSlotChanged.Broadcast();
}

bool UDC1PalPartyComponent::HasPalAt(int32 InSlotIndex) const
{
	return Pals.IsValidIndex(InSlotIndex) && Pals[InSlotIndex].IsValidPal();
}

int32 UDC1PalPartyComponent::GetPalCount() const
{
	int32 Count = 0;

	for (const FDC1PalInstance& Pal : Pals)
	{
		if (true == Pal.IsValidPal())
		{
			++Count;
		}
	}

	return Count;
}

void UDC1PalPartyComponent::HandlePalCaptured(const FDC1PalInstance& InPal)
{
	if (ROLE_Authority != GetOwnerRole() || false == InPal.IsValidPal())
	{
		return;
	}

	// 앞에서부터 빈 슬롯을 찾는다
	int32 EmptyIndex = INDEX_NONE;

	for (int32 Index = 0; Index < Pals.Num(); ++Index)
	{
		if (false == HasPalAt(Index))
		{
			EmptyIndex = Index;
			break;
		}
	}

	// TODO : 팰박스가 생기면?
	if (INDEX_NONE == EmptyIndex)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Party is full (%d). Captured pal was dropped."), Capacity);
		return;
	}

	Pals[EmptyIndex] = InPal;
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
	if (DeployedPal.IsValid())
	{
		RetrieveDeployed();
		return;
	}

	// 슬롯이 고정 길이라 인덱스만 봐서는 빈 칸을 걸러낼 수 없다
	if (false == HasPalAt(SelectedSlot))
	{
		UE_LOG(LogTemp, Log, TEXT("[Pal] Deploy: slot %d is empty."), SelectedSlot + 1);
		return;
	}

	if (false == Pals[SelectedSlot].IsDeployable())
	{
		UE_LOG(LogTemp, Log, TEXT("[Pal] Deploy: slot %d pal is down."), SelectedSlot + 1);
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
}

void UDC1PalPartyComponent::CycleSlot(int32 InDelta)
{
	if (0 == InDelta || Capacity <= 0)
	{
		return;
	}

	SelectedSlot = (SelectedSlot + InDelta + Capacity) % Capacity;

	LogSlot(SelectedSlot);

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

void UDC1PalPartyComponent::Server_SetSelectedSlot_Implementation(int32 InSlotIndex)
{
	SelectedSlot = FMath::Clamp(InSlotIndex, 0, Capacity - 1);
}

void UDC1PalPartyComponent::LogSlot(int32 InSlotIndex) const
{
	if (false == HasPalAt(InSlotIndex))
	{
		UE_LOG(LogTemp, Log, TEXT("[Pal] Slot %d/%d : empty"), InSlotIndex + 1, Capacity);
		return;
	}

	const FDC1PalInstance& Pal = Pals[InSlotIndex];

	UE_LOG(LogTemp, Log, TEXT("[Pal] Slot %d/%d : %s  HP %.0f%s"),
		InSlotIndex + 1, Capacity,
		*Pal.SpeciesTag.ToString(), Pal.CurrentHealth,
		Pal.IsDeployable() ? TEXT("") : TEXT("  (down)"));
}

void UDC1PalPartyComponent::RetrieveDeployed()
{
	GetWorld()->GetTimerManager().ClearTimer(DeathRetrieveTimer);

	ADC1PalCharacter* Deployed = DeployedPal.Get();
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
