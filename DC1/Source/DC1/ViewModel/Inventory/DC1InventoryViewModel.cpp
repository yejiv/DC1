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

const TArray<FDC1InventorySlotInfo>& UDC1InventoryViewModel::GetInventorySlots() const
{
	if (MyInventoryComp)
	{
		return MyInventoryComp->GetSlots();
	}

	static const TArray<FDC1InventorySlotInfo> EmptySlots;
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

const FDC1InventorySlotInfo& UDC1InventoryViewModel::GetSlot(int32 Index) const
{
	static const FDC1InventorySlotInfo EmptySlot;

	if (!MyInventoryComp)
	{
		return EmptySlot;
	}

	const TArray<FDC1InventorySlotInfo>& Slots = MyInventoryComp->GetSlots();
	if (Index < 0 || Index >= Slots.Num())
	{
		return EmptySlot;
	}

	return Slots[Index];
}

const TArray<FDC1InventorySlotInfo>& UDC1InventoryViewModel::GetChestInventorySlots() const
{
	if (TargetChestComp)
	{
		return TargetChestComp->GetSlots();
	}

	static const TArray<FDC1InventorySlotInfo> EmptySlots;
	return EmptySlots;
}

int32 UDC1InventoryViewModel::GetChestSlotCount() const
{
	if (TargetChestComp)
	{
		return TargetChestComp->GetSlots().Num();
	}
	return 0;
}

const FDC1InventorySlotInfo& UDC1InventoryViewModel::GetChestSlot(int32 Index) const
{
	static const FDC1InventorySlotInfo EmptySlot;

	if (!TargetChestComp)
	{
		return EmptySlot;
	}

	const TArray<FDC1InventorySlotInfo>& ChestSlots = TargetChestComp->GetSlots();
	if (Index < 0 || Index >= ChestSlots.Num())
	{
		return EmptySlot;
	}

	return ChestSlots[Index];
}

void UDC1InventoryViewModel::SetPlayerInventoryComponent(UDC1PlayerInventoryComponent* InPlayerComp)
{
	// 기존 Component 이벤트 구독 해제
	if (MyInventoryComp)
	{
		MyInventoryComp->OnInventoryUpdated.RemoveAll(this);
		MyInventoryComp->OnSlotChanged.RemoveAll(this);
	}

	MyInventoryComp = InPlayerComp;

	if (MyInventoryComp)
	{
		// 부분 갱신 이벤트 구독 (성능 최적화)
		MyInventoryComp->OnSlotChanged.AddUObject(this, &UDC1InventoryViewModel::HandleSlotUpdated);
		// 레거시 전체 갱신 이벤트도 유지 (필요시)
		MyInventoryComp->OnInventoryUpdated.AddUObject(this, &UDC1InventoryViewModel::HandleInventoryUpdated);
	}
}

void UDC1InventoryViewModel::SetTargetChest(UDC1ChestInventoryComponent* InChestComp)
{
	// 기존 Component 이벤트 구독 해제
	if (TargetChestComp)
	{
		TargetChestComp->OnInventoryUpdated.RemoveAll(this);
		TargetChestComp->OnSlotChanged.RemoveAll(this);
	}

	TargetChestComp = InChestComp;

	if (TargetChestComp)
	{
		// 부분 갱신 이벤트 구독 (성능 최적화) - 플레이어 쪽과 별도 델리게이트 사용
		TargetChestComp->OnSlotChanged.AddUObject(this, &UDC1InventoryViewModel::HandleChestSlotUpdated);
		// 레거시 전체 갱신 이벤트도 유지 (필요시)
		TargetChestComp->OnInventoryUpdated.AddUObject(this, &UDC1InventoryViewModel::HandleChestInventoryUpdated);
	}
}

void UDC1InventoryViewModel::RefreshSlot(int32 SlotIndex)
{
	OnSlotUIUpdated.Broadcast(SlotIndex);
}

void UDC1InventoryViewModel::HandleInventoryUpdated()
{
	// 이벤트 브로드캐스트 (View 업데이트 트리거)
	OnInventoryUpdated.Broadcast();
}

void UDC1InventoryViewModel::HandleSlotUpdated(int32 SlotIndex)
{
	// 특정 슬롯만 갱신 (부분 업데이트)
	OnSlotUIUpdated.Broadcast(SlotIndex);
}

void UDC1InventoryViewModel::HandleChestInventoryUpdated()
{
	OnChestInventoryUpdated.Broadcast();
}

void UDC1InventoryViewModel::HandleChestSlotUpdated(int32 SlotIndex)
{
	OnChestSlotUIUpdated.Broadcast(SlotIndex);
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

void UDC1InventoryViewModel::RequestMoveItem(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount /* = -1 */)
{
	if (!MyInventoryComp)
	{
		return;
	}

	UDC1PlayerInventoryComponent* PlayerInvComp = Cast<UDC1PlayerInventoryComponent>(MyInventoryComp);
	if (PlayerInvComp)
	{
		PlayerInvComp->RequestMoveItem(FromSlotIndex, ToSlotIndex, MoveCount);
	}
}

void UDC1InventoryViewModel::RequestMoveChestItem(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount /* = -1 */)
{
	if (!MyInventoryComp || !TargetChestComp)
	{
		return;
	}

	UDC1PlayerInventoryComponent* PlayerInvComp = Cast<UDC1PlayerInventoryComponent>(MyInventoryComp);
	if (PlayerInvComp)
	{
		PlayerInvComp->RequestMoveChestItem(TargetChestComp, FromSlotIndex, ToSlotIndex, MoveCount);
	}
}

void UDC1InventoryViewModel::RequestTransferItem(bool bToChest, int32 FromSlotIndex, int32 MoveCount /* = -1 */)
{
	if (!MyInventoryComp || !TargetChestComp)
	{
		return;
	}

	UDC1PlayerInventoryComponent* PlayerInvComp = Cast<UDC1PlayerInventoryComponent>(MyInventoryComp);
	if (PlayerInvComp)
	{
		PlayerInvComp->RequestTransferItem(TargetChestComp, bToChest, FromSlotIndex, MoveCount);
	}
}

// ========================
// 드래그 세션
// ========================

void UDC1InventoryViewModel::BeginDrag(bool bFromChest, int32 SlotIndex, int32 StackCount)
{
	bIsDragging = true;
	bDragSourceIsChest = bFromChest;
	DragSourceSlotIndex = SlotIndex;
	DragStackCount = StackCount;

	const FDC1InventorySlotInfo& SourceSlot = bFromChest ? GetChestSlot(SlotIndex) : GetSlot(SlotIndex);
	DragItemData = GetItemData(SourceSlot.ItemID);
}

void UDC1InventoryViewModel::EndDrag()
{
	bIsDragging = false;
	bDragSourceIsChest = false;
	DragSourceSlotIndex = -1;
	DragStackCount = 0;
	DragItemData = nullptr;

	OnDragEnded.Broadcast();
}

void UDC1InventoryViewModel::RequestDrop(bool bTargetIsChest, int32 TargetSlotIndex)
{
	if (!bIsDragging)
	{
		return;
	}

	if (bDragSourceIsChest == bTargetIsChest)
	{
		// 같은 인벤토리 내 이동 (플레이어 내부 또는 상자 내부)
		if (bDragSourceIsChest)
		{
			RequestMoveChestItem(DragSourceSlotIndex, TargetSlotIndex, DragStackCount);
		}
		else
		{
			RequestMoveItem(DragSourceSlotIndex, TargetSlotIndex, DragStackCount);
		}
	}
	else
	{
		// 플레이어 <-> 상자 이전 (bTargetIsChest가 곧 이동 방향)
		RequestTransferItem(bTargetIsChest, DragSourceSlotIndex, DragStackCount);
	}

	EndDrag();
}
