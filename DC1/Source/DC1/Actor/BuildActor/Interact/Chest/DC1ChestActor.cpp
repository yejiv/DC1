// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Interact/Chest/DC1ChestActor.h"

#include "Component/Inventory/DC1ChestInventoryComponent.h"
#include "Character/DC1Character.h"
#include "Character/Player/DC1PlayerController.h"
#include "UI/Tab/Inventory/Chest/DC1ChestLootScreenWidget.h"
#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Tags/UI/DC1GameplayTag_UI.h"
#include "System/DC1AssetManager.h"
#include "Data/DA/DataTable/DC1DataTableData.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

ADC1ChestActor::ADC1ChestActor()
{
	ChestInventoryComp = CreateDefaultSubobject<UDC1ChestInventoryComponent>("ChestInventoryComp");
}

void ADC1ChestActor::BeginPlay()
{
	Super::BeginPlay();

	if (true == ::IsValid(ChestInventoryComp))
	{
		ChestInventoryComp->SetCapacityAndColumns(ChestCapacity, ChestColumns);
	}

	AddDefaultItems();
}

void ADC1ChestActor::AddDefaultItems()
{
	if (false == HasAuthority() || false == ::IsValid(ChestInventoryComp))
	{
		return;
	}

	const UDC1DataTableData* DataTableAsset = UDC1AssetManager::GetAssetByTag<UDC1DataTableData>(DC1AssetTags::Asset_DataTableData);
	if (false == ::IsValid(DataTableAsset))
	{
		return;
	}

	const UDataTable* ItemDataTable = DataTableAsset->GetDataTable(DC1DataTableTags::DataTable_Item);
	if (false == ::IsValid(ItemDataTable))
	{
		return;
	}

	TArray<FDC1ItemRow*> AllRows;
	ItemDataTable->GetAllRows<FDC1ItemRow>(TEXT("ChestDefaultItems"), AllRows);

	const int32 AddTypeCount = FMath::Min(DefaultItemTypeCount, AllRows.Num());
	for (int32 i = 0; i < AddTypeCount; ++i)
	{
		if (nullptr == AllRows[i])
		{
			continue;
		}

		// 꺼내기 전용 상자(CanStoreItems == false)에는 TryStoreItem이 실패하므로 들어가지 않음
		ChestInventoryComp->TryStoreItem(AllRows[i]->ItemID, DefaultItemCount);
	}
}

void ADC1ChestActor::Interact(ADC1Character* InInteractOwner)
{
	if (false == ::IsValid(InInteractOwner) || false == ::IsValid(ChestInventoryComp))
	{
		return;
	}

	ADC1PlayerController* PC = Cast<ADC1PlayerController>(InInteractOwner->GetController());
	if (false == ::IsValid(PC))
	{
		return;
	}

	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		return;
	}

	UDC1ChestLootScreenWidget* LootScreen = Cast<UDC1ChestLootScreenWidget>(UIManager->PushScreen(DC1UITags::UI_Chest_Loot));
	if (false == ::IsValid(LootScreen))
	{
		return;
	}

	LootScreen->OpenChest(ChestInventoryComp);
}

bool ADC1ChestActor::StoreItem(FName InItemID, int32 InCount)
{
	if (false == HasAuthority() || false == ::IsValid(ChestInventoryComp))
	{
		return false;
	}

	return ChestInventoryComp->TryStoreItem(InItemID, InCount);
}

bool ADC1ChestActor::TakeItem(int32 InSlotIndex, int32 InCount)
{
	if (false == HasAuthority() || false == ::IsValid(ChestInventoryComp))
	{
		return false;
	}

	return ChestInventoryComp->RemoveItem(InSlotIndex, InCount);
}
