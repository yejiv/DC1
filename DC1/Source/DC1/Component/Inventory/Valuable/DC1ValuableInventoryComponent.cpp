// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Inventory/Valuable/DC1ValuableInventoryComponent.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Net/UnrealNetwork.h"

UDC1ValuableInventoryComponent::UDC1ValuableInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UDC1ValuableInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 플레이어 전용 데이터 - COND_OwnerOnly (플레이어 본인과 서버만)
	DOREPLIFETIME_CONDITION(UDC1ValuableInventoryComponent, Slots, COND_OwnerOnly);
}

bool UDC1ValuableInventoryComponent::AddItem(FName InItemID, int32 InCount /* = 1 */)
{
	// 서버에서만 실행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	if (InItemID.IsNone() || InCount <= 0)
	{
		return false;
	}

	if (false == CanAcceptItem(InItemID))
	{
		return false;
	}

	// 같은 아이템이 있으면 개수만 누적
	int32 SlotIndex = INDEX_NONE;
	if (FindItemInSlot(InItemID, &SlotIndex))
	{
		Slots[SlotIndex].StackCount += InCount;
		OnSlotChanged.Broadcast(SlotIndex);
		OnInventoryUpdated_Internal();
		return true;
	}

	// 처음 얻은 귀중품이면 슬롯을 하나 늘림
	FDC1InventorySlotInfo NewSlot;
	NewSlot.ItemID = InItemID;
	NewSlot.StackCount = InCount;

	const int32 NewSlotIndex = Slots.Add(NewSlot);
	OnSlotChanged.Broadcast(NewSlotIndex);
	OnInventoryUpdated_Internal();
	return true;
}

int32 UDC1ValuableInventoryComponent::AddItemPartial(FName InItemID, int32 InCount)
{
	if (InCount <= 0)
	{
		return 0;
	}

	return (true == AddItem(InItemID, InCount)) ? 0 : InCount;
}

bool UDC1ValuableInventoryComponent::RemoveItem(int32 InSlotIndex, int32 InCount /* = 1 */)
{
	// 서버에서만 실행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	if (!Slots.IsValidIndex(InSlotIndex) || InCount <= 0)
	{
		return false;
	}

	FDC1InventorySlotInfo& Slot = Slots[InSlotIndex];
	if (Slot.IsEmpty() || InCount > Slot.StackCount)
	{
		return false;
	}

	Slot.StackCount -= InCount;

	// 다 쓰면 슬롯 자체를 제거 (뒤 슬롯 인덱스가 당겨지므로 전체 갱신 이벤트로 처리)
	if (Slot.StackCount <= 0)
	{
		Slots.RemoveAt(InSlotIndex);
	}
	else
	{
		OnSlotChanged.Broadcast(InSlotIndex);
	}

	OnInventoryUpdated_Internal();
	return true;
}

bool UDC1ValuableInventoryComponent::CanAcceptItem(FName InItemID) const
{
	return IsValuableItem(InItemID);
}

void UDC1ValuableInventoryComponent::InitializeSlots()
{
	// 고정 칸이 없으므로 미리 만들 슬롯이 없음
	// 다른 컴포넌트의 BeginPlay(기본 아이템 지급)가 먼저 실행되어 이미 들어온 귀중품을 지우지 않도록 비우지 않는다
}

int32 UDC1ValuableInventoryComponent::GetStackLimit(const FDC1ItemRow& InItemRow) const
{
	// 귀중품은 스택 제한 없이 한 슬롯에 누적
	return MAX_int32;
}
