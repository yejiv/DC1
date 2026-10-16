// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Inventory/DC1InventoryViewModel.h"
#include "Component/Inventory/DC1InventoryComponentBase.h"
#include "Component/Inventory/Player/DC1PlayerInventoryComponent.h"
#include "Component/Inventory/Chest/DC1ChestInventoryComponent.h"
#include "Component/Inventory/Valuable/DC1ValuableInventoryComponent.h"
#include "Define/DC1ItemHelper.h"
#include "Character/Player/DC1PlayerState.h"
#include "Tags/Item/DC1GameplayTag_Item.h"

void UDC1InventoryViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	if (false == ::IsValid(InPlayerState))
	{
		return;
	}

	SetPlayerInventoryComponent(InPlayerState->GetInventoryComponent());
	SetValuableInventoryComponent(InPlayerState->GetValuableInventoryComponent());

	// 위젯 생성 순서와 무관하도록 구독 직후 현재 값을 한 번 알림
	OnInventoryUpdated.Broadcast();
	OnValuableInventoryUpdated.Broadcast();
	OnEquipmentUIUpdated.Broadcast();
}

void UDC1InventoryViewModel::ClearSource()
{
	// 드래그 중이었다면 세션을 닫아 드래그 복제본이 남지 않게 함
	if (bIsDragging)
	{
		EndDrag();
	}

	SetTargetChest(nullptr);
	SetPlayerInventoryComponent(nullptr);
	SetValuableInventoryComponent(nullptr);
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
		MyInventoryComp->OnEquipmentUpdated.RemoveAll(this);
	}

	MyInventoryComp = InPlayerComp;

	if (MyInventoryComp)
	{
		// 부분 갱신 이벤트 구독 (성능 최적화)
		MyInventoryComp->OnSlotChanged.AddUObject(this, &UDC1InventoryViewModel::HandleSlotUpdated);
		// 레거시 전체 갱신 이벤트도 유지 (필요시)
		MyInventoryComp->OnInventoryUpdated.AddUObject(this, &UDC1InventoryViewModel::HandleInventoryUpdated);
		MyInventoryComp->OnEquipmentUpdated.AddUObject(this, &UDC1InventoryViewModel::HandleEquipmentUpdated);
	}
}

int32 UDC1InventoryViewModel::GetValuableSlotCount() const
{
	if (ValuableInventoryComp)
	{
		return ValuableInventoryComp->GetSlots().Num();
	}
	return 0;
}

const FDC1InventorySlotInfo& UDC1InventoryViewModel::GetValuableSlot(int32 Index) const
{
	static const FDC1InventorySlotInfo EmptySlot;

	if (!ValuableInventoryComp)
	{
		return EmptySlot;
	}

	return ValuableInventoryComp->GetSlot(Index);
}

const FDC1InventorySlotInfo& UDC1InventoryViewModel::GetSlotBySource(EDC1InventorySource InSource, int32 Index) const
{
	switch (InSource)
	{
	case EDC1InventorySource::Valuable:
		return GetValuableSlot(Index);
	case EDC1InventorySource::Chest:
		return GetChestSlot(Index);
	case EDC1InventorySource::Equipment:
		return GetEquipmentSlot(Index);
	default:
		return GetSlot(Index);
	}
}

int32 UDC1InventoryViewModel::GetSlotCountBySource(EDC1InventorySource InSource) const
{
	switch (InSource)
	{
	case EDC1InventorySource::Valuable:
		return GetValuableSlotCount();
	case EDC1InventorySource::Chest:
		return GetChestSlotCount();
	case EDC1InventorySource::Equipment:
		return GetEquipmentSlotCount();
	default:
		return GetSlotCount();
	}
}

void UDC1InventoryViewModel::SetValuableInventoryComponent(UDC1ValuableInventoryComponent* InValuableComp)
{
	// 기존 Component 이벤트 구독 해제
	if (ValuableInventoryComp)
	{
		ValuableInventoryComp->OnInventoryUpdated.RemoveAll(this);
		ValuableInventoryComp->OnSlotChanged.RemoveAll(this);
	}

	ValuableInventoryComp = InValuableComp;

	if (ValuableInventoryComp)
	{
		ValuableInventoryComp->OnSlotChanged.AddUObject(this, &UDC1InventoryViewModel::HandleValuableSlotUpdated);
		ValuableInventoryComp->OnInventoryUpdated.AddUObject(this, &UDC1InventoryViewModel::HandleValuableInventoryUpdated);
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

void UDC1InventoryViewModel::HandleValuableInventoryUpdated()
{
	OnValuableInventoryUpdated.Broadcast();
}

void UDC1InventoryViewModel::HandleValuableSlotUpdated(int32 SlotIndex)
{
	OnValuableSlotUIUpdated.Broadcast(SlotIndex);
}

void UDC1InventoryViewModel::HandleEquipmentUpdated()
{
	OnEquipmentUIUpdated.Broadcast();
}

const FDC1ItemRow* UDC1InventoryViewModel::GetItemData(FName ItemID) const
{
	// 행 이름이 아니라 ItemID 필드로 조회
	return DC1ItemHelper::FindItemRow(ItemID);
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

void UDC1InventoryViewModel::RequestTransferItem(bool bToChest, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount /* = -1 */)
{
	if (!MyInventoryComp || !TargetChestComp)
	{
		return;
	}

	UDC1PlayerInventoryComponent* PlayerInvComp = Cast<UDC1PlayerInventoryComponent>(MyInventoryComp);
	if (PlayerInvComp)
	{
		PlayerInvComp->RequestTransferItem(TargetChestComp, bToChest, FromSlotIndex, ToSlotIndex, MoveCount);
	}
}

void UDC1InventoryViewModel::RequestSortInventory()
{
	if (!MyInventoryComp)
	{
		return;
	}

	MyInventoryComp->RequestSortInventory();
}

void UDC1InventoryViewModel::RequestSortChest()
{
	if (!MyInventoryComp || !TargetChestComp)
	{
		return;
	}

	// 상자는 소유자가 없어 직접 Server RPC를 보낼 수 없으므로 플레이어 인벤토리를 경유
	MyInventoryComp->RequestSortChestItems(TargetChestComp);
}

void UDC1InventoryViewModel::RequestSortValuables()
{
	if (!MyInventoryComp)
	{
		return;
	}

	// 기존 패턴과 동일하게 플레이어 인벤토리의 Server RPC를 경유
	MyInventoryComp->RequestSortValuables();
}

// ========================
// 장비
// ========================

int32 UDC1InventoryViewModel::GetEquipmentSlotCount() const
{
	if (false == ::IsValid(MyInventoryComp))
	{
		return 0;
	}

	return MyInventoryComp->GetEquipmentSlots().Num();
}

const FDC1InventorySlotInfo& UDC1InventoryViewModel::GetEquipmentSlot(int32 Index) const
{
	static const FDC1InventorySlotInfo EmptySlot;

	if (false == ::IsValid(MyInventoryComp))
	{
		return EmptySlot;
	}

	return MyInventoryComp->GetEquipmentSlot(Index);
}

EDC1EquipSlotType UDC1InventoryViewModel::GetEquipSlotType(int32 Index) const
{
	return DC1EquipSlotLayout::GetSlotType(Index);
}

bool UDC1InventoryViewModel::CanEquipToSlot(FName ItemID, int32 EquipSlotIndex) const
{
	if (false == ::IsValid(MyInventoryComp))
	{
		return false;
	}

	return MyInventoryComp->CanEquipToSlot(ItemID, EquipSlotIndex);
}

bool UDC1InventoryViewModel::IsEquipSlotLocked(int32 EquipSlotIndex) const
{
	if (false == ::IsValid(MyInventoryComp))
	{
		return true;
	}

	return false == MyInventoryComp->IsEquipSlotUnlocked(EquipSlotIndex);
}

float UDC1InventoryViewModel::GetCurrentWeight() const
{
	return ::IsValid(MyInventoryComp) ? MyInventoryComp->GetCurrentWeight() : 0.f;
}

float UDC1InventoryViewModel::GetMaxWeight() const
{
	return ::IsValid(MyInventoryComp) ? MyInventoryComp->GetMaxWeight() : 0.f;
}

int64 UDC1InventoryViewModel::GetMoneyCount() const
{
	if (false == ::IsValid(MyInventoryComp))
	{
		return 0;
	}

	int64 MoneyCount = 0;
	for (const FDC1InventorySlotInfo& Slot : MyInventoryComp->GetSlots())
	{
		if (true == Slot.IsEmpty())
		{
			continue;
		}

		const FDC1ItemRow* ItemRow = GetItemData(Slot.ItemID);
		if (nullptr != ItemRow && ItemRow->CategoryTag.MatchesTag(DC1ItemTags::Item_Money))
		{
			MoneyCount += Slot.StackCount;
		}
	}

	return MoneyCount;
}

void UDC1InventoryViewModel::RequestDropDraggedItem()
{
	if (false == bIsDragging)
	{
		return;
	}

	if (::IsValid(MyInventoryComp) && false == bDragSourceIsChest && false == bDragSourceIsEquipment)
	{
		MyInventoryComp->RequestDropItem(DragSourceSlotIndex, DragStackCount);
	}

	EndDrag();
}

void UDC1InventoryViewModel::RequestDiscardDraggedItem()
{
	if (false == bIsDragging)
	{
		return;
	}

	if (::IsValid(MyInventoryComp) && false == bDragSourceIsChest && false == bDragSourceIsEquipment)
	{
		MyInventoryComp->RequestDiscardItem(DragSourceSlotIndex, DragStackCount);
	}

	EndDrag();
}

void UDC1InventoryViewModel::RequestEquipItem(int32 SlotIndex, int32 EquipSlotIndex)
{
	if (false == ::IsValid(MyInventoryComp))
	{
		return;
	}

	MyInventoryComp->RequestEquipItem(SlotIndex, EquipSlotIndex);
}

void UDC1InventoryViewModel::RequestEquipItemAuto(int32 SlotIndex)
{
	if (false == ::IsValid(MyInventoryComp))
	{
		return;
	}

	MyInventoryComp->RequestEquipItemAuto(SlotIndex);
}

void UDC1InventoryViewModel::RequestUnequipItem(int32 EquipSlotIndex, int32 ToSlotIndex /* = -1 */)
{
	if (false == ::IsValid(MyInventoryComp))
	{
		return;
	}

	MyInventoryComp->RequestUnequipItem(EquipSlotIndex, ToSlotIndex);
}

// ========================
// 드래그 세션
// ========================

void UDC1InventoryViewModel::BeginDrag(bool bFromChest, int32 SlotIndex, int32 StackCount)
{
	bIsDragging = true;
	bDragSourceIsChest = bFromChest;
	bDragSourceIsEquipment = false;
	DragSourceSlotIndex = SlotIndex;
	DragStackCount = StackCount;

	const FDC1InventorySlotInfo& SourceSlot = bFromChest ? GetChestSlot(SlotIndex) : GetSlot(SlotIndex);
	DragItemData = GetItemData(SourceSlot.ItemID);
}

void UDC1InventoryViewModel::EndDrag()
{
	bIsDragging = false;
	bDragSourceIsChest = false;
	bDragSourceIsEquipment = false;
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

	// 장비 칸에서 시작한 드래그는 플레이어 인벤토리로만 해제 (상자로 직접 이동 불가)
	if (true == bDragSourceIsEquipment)
	{
		if (false == bTargetIsChest)
		{
			RequestUnequipItem(DragSourceSlotIndex, TargetSlotIndex);
		}

		EndDrag();
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
		// 플레이어 <-> 상자 이전 (bTargetIsChest가 곧 이동 방향, 드롭한 슬롯 기준으로 이동/병합/교환)
		RequestTransferItem(bTargetIsChest, DragSourceSlotIndex, TargetSlotIndex, DragStackCount);
	}

	EndDrag();
}

void UDC1InventoryViewModel::BeginDragFromEquipment(int32 EquipSlotIndex)
{
	const FDC1InventorySlotInfo& SourceSlot = GetEquipmentSlot(EquipSlotIndex);
	if (true == SourceSlot.IsEmpty())
	{
		return;
	}

	bIsDragging = true;
	bDragSourceIsChest = false;
	bDragSourceIsEquipment = true;
	DragSourceSlotIndex = EquipSlotIndex;
	DragStackCount = SourceSlot.StackCount;
	DragItemData = GetItemData(SourceSlot.ItemID);
}

void UDC1InventoryViewModel::RequestDropToEquipment(int32 EquipSlotIndex)
{
	if (false == bIsDragging)
	{
		return;
	}

	// 플레이어 인벤토리에서 시작한 드래그만 장착 (상자 -> 장비, 장비 -> 장비는 취소)
	if (false == bDragSourceIsChest && false == bDragSourceIsEquipment)
	{
		RequestEquipItem(DragSourceSlotIndex, EquipSlotIndex);
	}

	EndDrag();
}
