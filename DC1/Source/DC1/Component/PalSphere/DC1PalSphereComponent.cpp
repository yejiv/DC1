// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/PalSphere/DC1PalSphereComponent.h"

UDC1PalSphereComponent::UDC1PalSphereComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UDC1PalSphereComponent::CycleSphere()
{
	if (SphereDefs.Num() <= 0)
	{
		return;
	}

	SelectedSphereIndex = (SelectedSphereIndex + 1) % SphereDefs.Num();

	OnSphereSelectionChanged.Broadcast();
}

FName UDC1PalSphereComponent::GetSelectedSphereItemID() const
{
	return SphereDefs.IsValidIndex(SelectedSphereIndex) ? SphereDefs[SelectedSphereIndex].ItemID : NAME_None;
}

void UDC1PalSphereComponent::Server_SetThrowSphere_Implementation(FName InItemID)
{
	ThrowSphereItemID = InItemID;
}

const FDC1PalSphereDef* UDC1PalSphereComponent::FindSphereDef(FName InItemID) const
{
	return SphereDefs.FindByPredicate([InItemID](const FDC1PalSphereDef& Def) { return InItemID == Def.ItemID; });
}
