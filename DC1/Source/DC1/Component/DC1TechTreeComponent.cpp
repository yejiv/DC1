// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/DC1TechTreeComponent.h"
#include "Net/UnrealNetwork.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/DataTable/DC1DataTableData.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"

// Sets default values for this component's properties
UDC1TechTreeComponent::UDC1TechTreeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

int32 UDC1TechTreeComponent::GetPlayerLevel() const
{
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (false == ::IsValid(ASC))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1TechTreeComponent ::  Can't Find ASC"));
		return 0;
	}

	int32 NewLevel = FMath::TruncToInt32(ASC->GetNumericAttribute(UDC1AttributeSet::GetLevelAttribute()));

	return NewLevel;
}

EDC1TechSlotState UDC1TechTreeComponent::GetTechState(const FGameplayTag& InTechTag) const
{
	if (true == IsTechUnlocked(InTechTag))
	{
		return EDC1TechSlotState::Unlocked;
	}

	const FDC1TechRow* const* Found = TechRowsByTag.Find(InTechTag);
	if (nullptr == Found)
	{
		return EDC1TechSlotState::Default;
	}

	if (GetPlayerLevel() < (*Found)->RequiredLevel || TechPoints < (*Found)->UnlockCost)
	{
		return EDC1TechSlotState::Locked;
	}

	return EDC1TechSlotState::Available;
}

void UDC1TechTreeComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedTechRows();
	BindLevelChange();

	if (false == GetOwner()->HasAuthority())
	{
		return;
	}


	UnlockedTechTags.AppendTags(DefaultUnlockedTags);

	// Stand Alone 대응
	OnRep_TechState();
	OnRep_UnlockedTags();
}

void UDC1TechTreeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UDC1TechTreeComponent, TechPoints, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UDC1TechTreeComponent, AncientTechPoints, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UDC1TechTreeComponent, UnlockedTechTags, COND_OwnerOnly);
}


void UDC1TechTreeComponent::AddTechPoints(int32 InPoints)
{
	if (false == GetOwner()->HasAuthority())
	{
		return;
	}

	TechPoints += InPoints;

	//StandAlone
	OnRep_TechState();
}

void UDC1TechTreeComponent::AddAncientTechPoints(int32 InPoints)
{
	if (false == GetOwner()->HasAuthority())
	{
		return;
	}

	AncientTechPoints += InPoints;

	//StandAlone
	OnRep_TechState();
}

void UDC1TechTreeComponent::Server_UnlockTech_Implementation(const FGameplayTag& InTechTag)
{
	if (EDC1TechSlotState::Available != GetTechState(InTechTag))
	{
		return;
	}

	const FDC1TechRow* const* Found = TechRowsByTag.Find(InTechTag);
	if (nullptr == Found)
	{
		return;
	}

	TechPoints -= (*Found)->UnlockCost;

	UnlockTech(InTechTag);

	//StandAlone
	OnRep_TechState();
	OnRep_UnlockedTags();
}

void UDC1TechTreeComponent::Server_UnlockAncientTech_Implementation(const FGameplayTag& InTechTag)
{
	// GetAncient TechState로 추후 변경 ( GetTechState에서 TechPoint로 State 판별 )
	if (EDC1TechSlotState::Available != GetTechState(InTechTag))
	{
		return;
	}

	const FDC1TechRow* const* Found = TechRowsByTag.Find(InTechTag);
	if (nullptr == Found)
	{
		return;
	}

	AncientTechPoints -= (*Found)->UnlockCost;
	
	UnlockTech(InTechTag);

	//StandAlone
	OnRep_TechState();
}

void UDC1TechTreeComponent::OnRep_TechState()
{
	OnTechStateChange.Broadcast();
}

void UDC1TechTreeComponent::OnRep_UnlockedTags()
{
	for (const FGameplayTag& Tag : UnlockedTechTags)
	{
		if (false == LastUnlockedTags.HasTagExact(Tag))
		{
			OnTechUnlocked.Broadcast(Tag);
		}
	}

	LastUnlockedTags = UnlockedTechTags;
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

	TechRowsByTag.Empty();

	for (FDC1TechRow* Row : AllRows)
	{
		const FGameplayTag& TechTag = Row->TechTag;
		if (false == TechTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Tech Tag is Invalid"));
			continue;
		}

		if (nullptr != TechRowsByTag.Find(TechTag))
		{
			UE_LOG(LogTemp, Warning, TEXT("Duplication TechTag"));
			continue;
		}

		TechRowsByTag.Add(TechTag, Row);
	}
}

void UDC1TechTreeComponent::BindLevelChange()
{
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (false == ::IsValid(ASC))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1TechTreeComponent ::  Can't Find ASC"));
		return;
	}

	FOnGameplayAttributeValueChange& LevelChangeDelegate = ASC->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetLevelAttribute());

	LevelChangeDelegate.RemoveAll(this);
	LevelChangeDelegate.AddUObject(this, &ThisClass::HandleLevelChanged);
}

void UDC1TechTreeComponent::HandleLevelChanged(const FOnAttributeChangeData& InData)
{
	OnTechStateChange.Broadcast();
}

void UDC1TechTreeComponent::UnlockTech(const FGameplayTag& InTechTag)
{
	if (true == UnlockedTechTags.HasTagExact(InTechTag))
	{
		return;
	}

	const FDC1TechRow* const* Found = TechRowsByTag.Find(InTechTag);
	if (nullptr == Found)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1TechTreeComponent :: Cant Find TechRows by %s"), *InTechTag.ToString());
		return;
	}

	const FGameplayTag& TechTag = (*Found)->TechTag;

	UnlockedTechTags.AddTag(TechTag);
}
