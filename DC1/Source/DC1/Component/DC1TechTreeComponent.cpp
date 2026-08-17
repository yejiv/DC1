// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/DC1TechTreeComponent.h"
#include "Net/UnrealNetwork.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/DataTable/DC1DataTableData.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

// Sets default values for this component's properties
UDC1TechTreeComponent::UDC1TechTreeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

EDC1TechSlotState UDC1TechTreeComponent::GetTechState(const FName& TechID) const
{
	if (true == IsTechUnlocked(TechID))
	{
		return EDC1TechSlotState::Unlocked;
	}

	const FDC1TechRow* const* Found = TechRowsByID.Find(TechID);
	if (nullptr == Found)
	{
		return EDC1TechSlotState::Default;
	}

	if (TechPoints < (*Found)->UnlockCost)
	{
		return EDC1TechSlotState::Locked;
	}

	// LEVEL 검사 ( AttributeSet )

	return EDC1TechSlotState::Available;
}

void UDC1TechTreeComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedTechRows();
}

void UDC1TechTreeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UDC1TechTreeComponent, UnlockedTechIDs, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UDC1TechTreeComponent, TechPoints, COND_OwnerOnly);
}

void UDC1TechTreeComponent::AddTechPoints(int32 Points)
{
	if (false == GetOwner()->HasAuthority())
	{
		return;
	}

	TechPoints += Points;

	//StandAlone
	OnRep_TechState();
}

void UDC1TechTreeComponent::Server_UnlockTech_Implementation(const FName& TechID)
{
	if (EDC1TechSlotState::Available != GetTechState(TechID))
	{
		return;
	}

	const FDC1TechRow* const* Found = TechRowsByID.Find(TechID);

	TechPoints -= (*Found)->UnlockCost;
	UnlockedTechIDs.Add(TechID);

	//StandAlone
	OnRep_TechState();
}

void UDC1TechTreeComponent::OnRep_TechState()
{
	OnTechStateChange.Broadcast();
}

void UDC1TechTreeComponent::CachedTechRows()
{
	const UDC1DataTableData* DataTable = UDC1AssetManager::GetAssetByTag<UDC1DataTableData>(DC1AssetTags::Asset_DataTableData);
	if (false == ::IsValid(DataTable))
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Find DataTable Asset from AssetManager"));
		return;
	}

	CachedTechData = DataTable->GetDataTable(DC1DataTableTags::DataTable_UI_TechTree);
	if (false == ::IsValid(CachedTechData))
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Find TechTree DataTable from DataTableAsset"));
		return;
	}

	TArray<FDC1TechRow*> AllRows;
	CachedTechData->GetAllRows<FDC1TechRow>(TEXT("Cached Tech Rows"), AllRows);
	if (true == AllRows.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("TechTree DataTable is NULL"));
		return;
	}

	TechRowsByID.Empty();

	for (FDC1TechRow* Row : AllRows)
	{
		FName TechID = Row->TechID;
		if (true == TechID.IsNone())
		{
			UE_LOG(LogTemp, Warning, TEXT("TechID is None"));
			continue;
		}

		if (nullptr != TechRowsByID.Find(TechID))
		{
			UE_LOG(LogTemp, Warning, TEXT("Duplication TechID"));
			continue;
		}

		TechRowsByID.Add(TechID, Row);
	}
}