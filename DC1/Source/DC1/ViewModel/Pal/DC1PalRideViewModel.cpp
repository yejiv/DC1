// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Pal/DC1PalRideViewModel.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Component/Pal/DC1PalRideComponent.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

void UDC1PalRideViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	if (nullptr == InPlayerState)
		return;

	PalPartyComponent = InPlayerState->GetPalPartyComponent();

	if (false == PalPartyComponent.IsValid())
		return;

	PalPartyComponent->OnPalSlotChanged.AddUObject(this, &ThisClass::HandleDeployedPalChanged);

	HandleDeployedPalChanged();
}

void UDC1PalRideViewModel::ClearSource()
{
	if (true == RideComponent.IsValid())
	{
		RideComponent->OnRideStateChanged.RemoveAll(this);
		RideComponent.Reset();
	}

	if (true == PalPartyComponent.IsValid())
	{
		PalPartyComponent->OnPalSlotChanged.RemoveAll(this);
		PalPartyComponent.Reset();
	}

	bRideAvailable = false;
	bRiding = false;
	RideSkillNames.Reset();
}

void UDC1PalRideViewModel::SetRideHoldPressed(bool InPressed)
{
	OnRideHoldChanged.Broadcast(InPressed);
}

void UDC1PalRideViewModel::NotifyRideHoldCompleted()
{
	OnRideHoldCompleted.Broadcast();
}

void UDC1PalRideViewModel::HandleDeployedPalChanged()
{
	if (false == PalPartyComponent.IsValid())
		return;

	const ADC1PalCharacter* DeployedPal = PalPartyComponent->GetDeployedPal();
	UDC1PalRideComponent* NewRideComponent = (nullptr != DeployedPal) ? DeployedPal->GetRideComponent() : nullptr;

	if (RideComponent.Get() != NewRideComponent)
	{
		if (true == RideComponent.IsValid())
		{
			RideComponent->OnRideStateChanged.RemoveAll(this);
		}

		RideComponent = NewRideComponent;

		if (true == RideComponent.IsValid())
		{
			RideComponent->OnRideStateChanged.AddUObject(this, &ThisClass::HandleRideStateChanged);
		}
	}

	const UDC1PalData* PalData = (nullptr != DeployedPal) ? DeployedPal->GetPalData() : nullptr;
	bRideAvailable = (nullptr != PalData) && (true == PalData->bRideable) && (true == RideComponent.IsValid());

	RefreshRideSkillNames(bRideAvailable ? PalData : nullptr);

	HandleRideStateChanged();
}

void UDC1PalRideViewModel::HandleRideStateChanged()
{
	bRiding = (true == bRideAvailable) && (EDC1RideState::None != RideComponent->GetRideState());

	OnRideInfoChanged.Broadcast();
}

void UDC1PalRideViewModel::RefreshRideSkillNames(const UDC1PalData* InPalData)
{
	RideSkillNames.Reset();

	if (nullptr == InPalData)
		return;

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == IsValid(Registry))
		return;

	// 순서가 입력 순서라 정의가 없어도 자리는 남긴다
	for (const FGameplayTag& SkillTag : InPalData->SkillTags)
	{
		const FDC1PalSkillDef* SkillDef = Registry->FindSkillDef(SkillTag);
		RideSkillNames.Add((nullptr != SkillDef) ? SkillDef->DisplayName : FText::GetEmpty());
	}
}
