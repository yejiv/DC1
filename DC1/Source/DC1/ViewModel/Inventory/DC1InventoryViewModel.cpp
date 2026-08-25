// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Inventory/DC1InventoryViewModel.h"
#include "Component/Inventory/DC1InventoryComponentBase.h"
#include "Component/Inventory/DC1PlayerInventoryComponent.h"
#include "Component/Inventory/DC1ChestInventoryComponent.h"
#include "System/DC1AssetManager.h"
#include "Data/DA/DataTable/DC1DataTableData.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

UDC1InventoryViewModel::UDC1InventoryViewModel()
{
}

const TArray<FDC1InventorySlot>& UDC1InventoryViewModel::GetInventorySlots() const
{
	if (MyInventoryComp)
	{
		return MyInventoryComp->GetSlots();
	}

	static const TArray<FDC1InventorySlot> EmptySlots;
	return EmptySlots;
}

int32 UDC1InventoryViewModel::GetSlotCount() const
{
	if (MyInventoryComp)
	{
		return MyInventoryComp->GetSlots().Num();
	}
	return 0;
}

const FDC1InventorySlot& UDC1InventoryViewModel::GetSlot(int32 Index) const
{
	static const FDC1InventorySlot EmptySlot;

	if (!MyInventoryComp)
	{
		return EmptySlot;
	}

	const TArray<FDC1InventorySlot>& Slots = MyInventoryComp->GetSlots();
	if (Index < 0 || Index >= Slots.Num())
	{
		return EmptySlot;
	}

	return Slots[Index];
}

void UDC1InventoryViewModel::SetPlayerInventoryComponent(UDC1PlayerInventoryComponent* InPlayerComp)
{
	MyInventoryComp = InPlayerComp;

	if (MyInventoryComp)
	{
		// 인벤토리 변경 이벤트 구독
		MyInventoryComp->OnInventoryUpdated.AddUObject(this, &UDC1InventoryViewModel::HandleInventoryUpdated);
	}
}

void UDC1InventoryViewModel::SetTargetChest(UDC1ChestInventoryComponent* InChestComp)
{
	TargetChestComp = InChestComp;
}

void UDC1InventoryViewModel::HandleInventoryUpdated()
{
	// 이벤트 브로드캐스트 (View 업데이트 트리거)
	OnInventoryUpdated.Broadcast();
}

const FDC1ItemRow* UDC1InventoryViewModel::GetItemData(FName ItemID) const
{
	if (ItemID.IsNone())
	{
		return nullptr;
	}

	const UDC1DataTableData* DataTableAsset = UDC1AssetManager::GetAssetByTag<UDC1DataTableData>(DC1AssetTags::Asset_DataTableData);
	if (!IsValid(DataTableAsset))
	{
		return nullptr;
	}

	const UDataTable* ItemDataTable = DataTableAsset->GetDataTable(DC1DataTableTags::DataTable_Item);
	if (!IsValid(ItemDataTable))
	{
		return nullptr;
	}

	return ItemDataTable->FindRow<FDC1ItemRow>(ItemID, TEXT("GetItemData"));
}
