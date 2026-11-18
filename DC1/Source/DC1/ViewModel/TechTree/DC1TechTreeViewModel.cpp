// Fill out your copyright notice in the Description page of Project Settings.


#include "ViewModel/TechTree/DC1TechTreeViewModel.h"

#include "Character/Player/DC1PlayerState.h"
#include "Data/DataTable/DC1DataTableTypes.h"

void UDC1TechTreeViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	if (false == ::IsValid(InPlayerState))
	{
		return;
	}

	SetTechTreeComponent(InPlayerState->GetTechTreeComponent());
}

void UDC1TechTreeViewModel::ClearSource()
{
	SetTechTreeComponent(nullptr);
}

void UDC1TechTreeViewModel::SetTechTreeComponent(UDC1TechTreeComponent* InTechComp)
{
	if (TechComp == InTechComp)
	{
		return;
	}

	if (true == ::IsValid(TechComp))
	{
		TechComp->OnTechStateChange.RemoveAll(this);
		TechComp->OnTechUnlocked.RemoveAll(this);
	}

	TechComp = InTechComp;

	if (true == ::IsValid(TechComp))
	{
		TechComp->OnTechStateChange.AddUObject(this, &ThisClass::HandleTechStateChanged);
		TechComp->OnTechUnlocked.AddUObject(this, &ThisClass::HandleTechUnlocked);
	}

	OnTechStateChanged.Broadcast();
}

int32 UDC1TechTreeViewModel::GetTechPoints() const
{
	return ::IsValid(TechComp) ? TechComp->GetTechPoints() : 0;
}

int32 UDC1TechTreeViewModel::GetAncientTechPoints() const
{
	return ::IsValid(TechComp) ? TechComp->GetAncientTechPoints() : 0;
}

int32 UDC1TechTreeViewModel::GetPlayerLevel() const
{
	return ::IsValid(TechComp) ? TechComp->GetPlayerLevel() : 0;
}

EDC1TechSlotState UDC1TechTreeViewModel::GetTechState(const FGameplayTag& InTechTag) const
{
	return ::IsValid(TechComp) ? TechComp->GetTechState(InTechTag) : EDC1TechSlotState::Default;
}

const TMap<FGameplayTag, const FDC1TechRow>& UDC1TechTreeViewModel::GetTechRowsByTag() const
{
	static const TMap<FGameplayTag, const FDC1TechRow> EmptyRows;

	return ::IsValid(TechComp) ? TechComp->GetTechRowsByTag() : EmptyRows;
}

void UDC1TechTreeViewModel::RequestUnlockTech(const FGameplayTag& InTechTag)
{
	if (true == ::IsValid(TechComp))
	{
		TechComp->RequestUnlockTech(InTechTag);
	}
}

void UDC1TechTreeViewModel::RequestUnlockAncientTech(const FGameplayTag& InTechTag)
{
	if (true == ::IsValid(TechComp))
	{
		TechComp->RequestUnlockAncientTech(InTechTag);
	}
}

void UDC1TechTreeViewModel::HandleTechStateChanged()
{
	OnTechStateChanged.Broadcast();
}

void UDC1TechTreeViewModel::HandleTechUnlocked(const FGameplayTag& InTechTag)
{
	OnTechUnlocked.Broadcast(InTechTag);
}
