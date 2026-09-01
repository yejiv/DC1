// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Pal/DC1PalPartyComponent.h"

#include "Character/Pal/DC1PalCharacter.h"

#include "GameFramework/PlayerState.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"

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

void UDC1PalPartyComponent::HandlePalCaptured(const FDC1PalInstance& InPal)
{
	if (ROLE_Authority != GetOwnerRole() || false == InPal.IsValidPal())
	{
		return;
	}

	// TODO : 팰박스가 생기면 파티가 꽉 찼을 때 그쪽으로 넘긴다
	if (Pals.Num() >= Capacity)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Party is full (%d). Captured pal was dropped."), Capacity);
		return;
	}

	Pals.Add(InPal);
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

	// 꺼내둔 게 있으면 접는다. Fold가 액터를 파괴하므로 반환값으로 목록을 갱신해야
	// 싸우다 깎인 체력이 남는다
	if (ADC1PalCharacter* Deployed = DeployedPal.Get())
	{
		UpdatePal(Deployed->Fold());
		DeployedPal = nullptr;
		return;
	}

	if (0 == Pals.Num())
	{
		return;
	}

	FTransform SpawnTransform;
	if (false == FindDeployTransform(SpawnTransform))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Deploy: no navigable spot near the owner."));
		return;
	}

	// TODO : 파티 슬롯 UI가 생기면 첫 마리가 아니라 선택된 팰을 꺼낸다
	DeployedPal = ADC1PalCharacter::SpawnFromInstanceData(GetWorld(), Pals[0], SpawnTransform);
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
