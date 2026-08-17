// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Inventory/DC1InventoryViewModel.h"

UDC1InventoryViewModel::UDC1InventoryViewModel()
{
	InitializeMockData();
}

const FDC1InventorySlot& UDC1InventoryViewModel::GetSlot(int32 Index) const
{
	static const FDC1InventorySlot EmptySlot;
	if (Index < 0 || Index >= InventorySlots.Num())
	{
		return EmptySlot;
	}
	return InventorySlots[Index];
}

const FDC1ItemRow* UDC1InventoryViewModel::GetItemData(const FName& ItemID) const
{
	return ItemDatabase.Find(ItemID);
}

void UDC1InventoryViewModel::InitializeMockData()
{
	// 슬롯 초기화 (5x7 = 35개)
	InventorySlots.Reserve(35);
	for (int32 i = 0; i < 35; ++i)
	{
		InventorySlots.Add(FDC1InventorySlot());
	}
}
