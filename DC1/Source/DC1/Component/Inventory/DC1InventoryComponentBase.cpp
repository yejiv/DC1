// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Inventory/DC1InventoryComponentBase.h"
#include "Net/UnrealNetwork.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1ItemHelper.h"
#include "Define/DC1Macro.h"

UDC1InventoryComponentBase::UDC1InventoryComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UDC1InventoryComponentBase::BeginPlay()
{
	Super::BeginPlay();

	// 슬롯 초기화 (클라이언트에서도 실행되지만 복제된 데이터로 덮어씌워짐)
	if (GetOwnerRole() == ROLE_Authority)
	{
		InitializeSlots();
	}
}

void UDC1InventoryComponentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Slots의 복제 조건은 용도에 따라 다르므로(플레이어: 본인만 / 상자: 전체 공개)
	// 자식 클래스(DC1PlayerInventoryComponent, DC1ChestInventoryComponent)에서 각자 등록함
}

const FDC1InventorySlotInfo& UDC1InventoryComponentBase::GetSlot(int32 InIndex) const
{
	static const FDC1InventorySlotInfo EmptySlot = FDC1InventorySlotInfo();

	if (!Slots.IsValidIndex(InIndex))
	{
		return EmptySlot;
	}

	return Slots[InIndex];
}

bool UDC1InventoryComponentBase::IsSlotEmpty(int32 InIndex) const
{
	if (!Slots.IsValidIndex(InIndex))
	{
		return true;
	}

	return Slots[InIndex].IsEmpty();
}

bool UDC1InventoryComponentBase::IsInventoryFull() const
{
	for (const FDC1InventorySlotInfo& Slot : Slots)
	{
		if (Slot.IsEmpty())
		{
			return false;
		}
	}

	return true;
}

int32 UDC1InventoryComponentBase::FindEmptySlot() const
{
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i].IsEmpty())
		{
			return i;
		}
	}

	return -1;
}

bool UDC1InventoryComponentBase::FindItemInSlot(FName InItemID, OUT int32* SlotIndex) const
{
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i].ItemID.IsEqual(InItemID))
		{
			if (SlotIndex)
				*SlotIndex = i;

			return true;
		}
	}

	return false;
}

int32 UDC1InventoryComponentBase::GetItemCount(FName InItemID) const
{
	int32 StackCount = 0;

	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i].ItemID.IsEqual(InItemID))
		{
			StackCount += Slots[i].StackCount;
		}
	}

	return StackCount;
}

bool UDC1InventoryComponentBase::RemoveItemByID(FName InItemID, int32 InCount)
{
	if (GetOwnerRole() != ROLE_Authority || InItemID == NAME_None || InCount <= 0)
	{
		return false;
	}

	int32 TotalStackCount = GetItemCount(InItemID);
	if (TotalStackCount < InCount)
	{
		return false;
	}

	int32 TotalRemoveCount = InCount;

	for (int32 i = Slots.Num() - 1; i >= 0; --i)
	{
		if (Slots[i].ItemID.IsEqual(InItemID))
		{
			int32 RemoveCount = FMath::Min(TotalRemoveCount, Slots[i].StackCount);
			RemoveItem(i, RemoveCount);

			TotalRemoveCount -= RemoveCount;
		}

		if (TotalRemoveCount <= 0)
		{
			break;
		}
	}

	return true;
}


bool UDC1InventoryComponentBase::AddItem(FName InItemID, int32 InCount)
{
	// 서버에서만 실행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	// 유효성 검사
	if (InItemID.IsNone() || InCount <= 0)
	{
		return false;
	}

	// 이 인벤토리에 보관할 수 없는 아이템 (예: 상자에 귀중품)
	if (false == CanAcceptItem(InItemID))
	{
		return false;
	}

	int32 SlotIndex = INDEX_NONE;

	if (FindItemInSlot(InItemID, &SlotIndex))
	{
		// 기존 수량에 누적 (덮어쓰기 방지)
		Slots[SlotIndex].StackCount += InCount;
		OnSlotChanged.Broadcast(SlotIndex);
		OnInventoryUpdated_Internal();
		return true;
	}

	// 새 슬롯에 추가
	int32 NewSlotIndex = FindEmptySlot();
	if (NewSlotIndex != INDEX_NONE)
	{
		Slots[NewSlotIndex].ItemID = InItemID;
		Slots[NewSlotIndex].StackCount = InCount;
		OnSlotChanged.Broadcast(NewSlotIndex);
		OnInventoryUpdated_Internal();
		return true;
	}

	return false;
}

bool UDC1InventoryComponentBase::RemoveItem(int32 InSlotIndex, int32 InCount)
{
	// 서버에서만 실행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	// 유효성 검사
	if (!Slots.IsValidIndex(InSlotIndex) || InCount <= 0)
	{
		return false;
	}

	FDC1InventorySlotInfo& Slot = Slots[InSlotIndex];

	// 슬롯이 비어있으면 실패
	if (Slot.IsEmpty())
	{
		return false;
	}

	// 제거할 개수가 현재 개수보다 많으면 실패
	if (InCount > Slot.StackCount)
	{
		return false;
	}

	// 아이템 감소
	Slot.StackCount -= InCount;

	// 개수가 0이면 슬롯 비우기
	if (Slot.StackCount <= 0)
	{
		Slot.Clear();
	}

	OnSlotChanged.Broadcast(InSlotIndex);
	OnInventoryUpdated_Internal();
	return true;
}

void UDC1InventoryComponentBase::SwapSlots(int32 InIndex1, int32 InIndex2)
{
	// 서버에서만 실행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	// 유효성 검사
	if (!Slots.IsValidIndex(InIndex1) || !Slots.IsValidIndex(InIndex2))
	{
		return;
	}

	// 같은 슬롯이면 무시
	if (InIndex1 == InIndex2)
	{
		return;
	}

	// 슬롯 교환
	Swap(Slots[InIndex1], Slots[InIndex2]);

	// 두 슬롯 모두 변경 알림
	OnSlotChanged.Broadcast(InIndex1);
	OnSlotChanged.Broadcast(InIndex2);
	OnInventoryUpdated_Internal();
}

void UDC1InventoryComponentBase::MoveOrMergeSlot(int32 FromIndex, int32 ToIndex, int32 MoveCount /* = -1 */)
{
	// 서버에서만 실행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	if (!Slots.IsValidIndex(FromIndex) || !Slots.IsValidIndex(ToIndex))
	{
		return;
	}

	if (FromIndex == ToIndex)
	{
		return;
	}

	FDC1InventorySlotInfo SourceSlot = Slots[FromIndex];
	if (SourceSlot.IsEmpty())
	{
		return;
	}

	// MoveCount가 -1이면 전체 이동, 아니면 지정된 개수만 이동
	int32 ActualMoveCount = (MoveCount <= 0) ? SourceSlot.StackCount : FMath::Min(MoveCount, SourceSlot.StackCount);

	FDC1InventorySlotInfo& TargetSlot = Slots[ToIndex];

	if (TargetSlot.IsEmpty())
	{
		// 빈 슬롯으로 이동
		TargetSlot.ItemID = SourceSlot.ItemID;
		TargetSlot.StackCount = ActualMoveCount;
		Slots[FromIndex].StackCount -= ActualMoveCount;

		if (Slots[FromIndex].StackCount <= 0)
		{
			Slots[FromIndex].Clear();
		}
	}
	else if (TargetSlot.ItemID == SourceSlot.ItemID)
	{
		// 같은 아이템이 있으면 병합
		TargetSlot.StackCount += ActualMoveCount;
		Slots[FromIndex].StackCount -= ActualMoveCount;

		if (Slots[FromIndex].StackCount <= 0)
		{
			Slots[FromIndex].Clear();
		}
	}
	else
	{
		// 다른 아이템이 있으면 교환 (이동 개수 무시, 전체 교환)
		Swap(Slots[FromIndex], Slots[ToIndex]);
	}

	OnSlotChanged.Broadcast(FromIndex);
	OnSlotChanged.Broadcast(ToIndex);
	OnInventoryUpdated_Internal();
}

bool UDC1InventoryComponentBase::TransferSlotTo(UDC1InventoryComponentBase* InTargetComp, int32 FromIndex, int32 ToIndex, int32 MoveCount /* = -1 */, bool bAllowSwap /* = true */)
{
	// 서버에서만 실행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	// 같은 인벤토리 내부 이동은 MoveOrMergeSlot 사용
	if (false == ::IsValid(InTargetComp) || this == InTargetComp)
	{
		return false;
	}

	if (!Slots.IsValidIndex(FromIndex) || !InTargetComp->Slots.IsValidIndex(ToIndex))
	{
		return false;
	}

	FDC1InventorySlotInfo& SourceSlot = Slots[FromIndex];
	if (SourceSlot.IsEmpty())
	{
		return false;
	}

	// 목표 인벤토리가 받을 수 없는 아이템 (예: 귀중품 -> 상자)
	if (false == InTargetComp->CanAcceptItem(SourceSlot.ItemID))
	{
		return false;
	}

	// MoveCount가 -1이면 전체 이동, 아니면 지정된 개수만 이동
	const int32 ActualMoveCount = (MoveCount <= 0) ? SourceSlot.StackCount : FMath::Min(MoveCount, SourceSlot.StackCount);

	FDC1InventorySlotInfo& TargetSlot = InTargetComp->Slots[ToIndex];

	// 교환 시에는 목표 슬롯의 아이템이 이쪽으로 넘어오므로 이쪽도 받을 수 있어야 함
	const bool bIsSwap = (false == TargetSlot.IsEmpty() && TargetSlot.ItemID != SourceSlot.ItemID);
	if (true == bIsSwap && false == CanAcceptItem(TargetSlot.ItemID))
	{
		return false;
	}

	if (TargetSlot.IsEmpty() || TargetSlot.ItemID == SourceSlot.ItemID)
	{
		// 빈 슬롯이면 이동, 같은 아이템이면 병합
		if (TargetSlot.IsEmpty())
		{
			TargetSlot.ItemID = SourceSlot.ItemID;
			TargetSlot.StackCount = 0;
		}

		TargetSlot.StackCount += ActualMoveCount;
		SourceSlot.StackCount -= ActualMoveCount;

		if (SourceSlot.StackCount <= 0)
		{
			SourceSlot.Clear();
		}
	}
	else
	{
		if (false == bAllowSwap)
		{
			return false;
		}

		// 다른 아이템이 있으면 교환 (이동 개수 무시, 전체 교환)
		Swap(SourceSlot, TargetSlot);
	}

	OnSlotChanged.Broadcast(FromIndex);
	OnInventoryUpdated_Internal();

	InTargetComp->OnSlotChanged.Broadcast(ToIndex);
	InTargetComp->OnInventoryUpdated_Internal();

	return true;
}

void UDC1InventoryComponentBase::ClearSlot(int32 InSlotIndex)
{
	// 서버에서만 실행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	// 유효성 검사
	if (!Slots.IsValidIndex(InSlotIndex))
	{
		return;
	}

	// 이미 비어있으면 무시
	if (Slots[InSlotIndex].IsEmpty())
	{
		return;
	}

	// 슬롯 비우기
	Slots[InSlotIndex].Clear();

	OnSlotChanged.Broadcast(InSlotIndex);
	OnInventoryUpdated_Internal();
}

void UDC1InventoryComponentBase::SetCapacityAndColumns(int32 InTotalCapacity, int32 InColumns)
{
	// 서버에서만 실행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	// 유효성 검사
	if (InTotalCapacity <= 0 || InColumns <= 0)
	{
		return;
	}

	// 기존 데이터 보존하며 크기 조정
	if (InTotalCapacity < Slots.Num())
	{
		// 용량이 줄어드는 경우, 초과한 슬롯 제거
		Slots.RemoveAt(InTotalCapacity, Slots.Num() - InTotalCapacity);
	}
	else if (InTotalCapacity > Slots.Num())
	{
		// 용량이 늘어나는 경우, 빈 슬롯 추가
		Slots.Reserve(InTotalCapacity);
		for (int32 i = Slots.Num(); i < InTotalCapacity; ++i)
		{
			Slots.Add(FDC1InventorySlotInfo());
		}
	}

	TotalCapacity = InTotalCapacity;
	Columns = InColumns;
	Rows = (TotalCapacity + Columns - 1) / Columns; // 올림 나누기

	OnInventoryUpdated_Internal();
}

void UDC1InventoryComponentBase::InitializeSlots()
{
	// 기존 슬롯 비우기
	Slots.Empty();

	// Rows 자동 계산 (올림 나누기)
	Rows = (TotalCapacity + Columns - 1) / Columns;

	// 새 슬롯 생성
	Slots.Reserve(TotalCapacity);

	for (int32 i = 0; i < TotalCapacity; ++i)
	{
		Slots.Add(FDC1InventorySlotInfo());
	}

	// UE_LOG(LogTemp, Warning, TEXT("[%s] Inventory initialized: %d slots (%d x %d)"),
	//	*GetOwner()->GetName(), TotalCapacity, Rows, Columns);
}

void UDC1InventoryComponentBase::SortSlots()
{
	// 서버에서만 실행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	// 1. 같은 아이템끼리 수량 합산 (DataTable에 없는 아이템은 병합하지 않고 따로 보관)
	TMap<FName, int32> TotalCountByItem;
	TArray<FDC1InventorySlotInfo> UnknownSlots;

	for (const FDC1InventorySlotInfo& Slot : Slots)
	{
		if (Slot.IsEmpty())
		{
			continue;
		}

		if (nullptr == DC1ItemHelper::FindItemRow(Slot.ItemID))
		{
			UnknownSlots.Add(Slot);
			continue;
		}

		TotalCountByItem.FindOrAdd(Slot.ItemID) += Slot.StackCount;
	}

	// 2. ItemID -> 레어도 순 정렬
	TArray<FName> SortedItemIDs;
	TotalCountByItem.GetKeys(SortedItemIDs);

	SortedItemIDs.Sort([](const FName& A, const FName& B)
	{
		const int32 CompareResult = A.Compare(B);
		if (0 != CompareResult)
		{
			return CompareResult < 0;
		}

		const FDC1ItemRow* RowA = DC1ItemHelper::FindItemRow(A);
		const FDC1ItemRow* RowB = DC1ItemHelper::FindItemRow(B);
		return RowA->ItemRarity < RowB->ItemRarity;
	});

	// 3. MaxStack 단위로 나눠 앞쪽 슬롯부터 채움
	TArray<FDC1InventorySlotInfo> SortedSlots;
	SortedSlots.Reserve(Slots.Num());

	for (const FName& ItemID : SortedItemIDs)
	{
		const FDC1ItemRow* ItemRow = DC1ItemHelper::FindItemRow(ItemID);
		const int32 MaxStack = FMath::Max(1, GetStackLimit(*ItemRow));

		int32 RemainCount = TotalCountByItem[ItemID];
		while (RemainCount > 0)
		{
			FDC1InventorySlotInfo NewSlot;
			NewSlot.ItemID = ItemID;
			NewSlot.StackCount = FMath::Min(RemainCount, MaxStack);
			SortedSlots.Add(NewSlot);

			RemainCount -= NewSlot.StackCount;
		}
	}

	SortedSlots.Append(UnknownSlots);

	if (true == IsFixedCapacity())
	{
		// 기존에 MaxStack을 넘긴 스택이 있었다면 분할 후 칸이 부족할 수 있음 - 아이템 유실 방지를 위해 정렬 취소
		if (SortedSlots.Num() > Slots.Num())
		{
			LOG_WARNING(TEXT("[SortSlots] 정렬 후 필요한 칸(%d)이 용량(%d)을 넘어 정렬을 취소함"), SortedSlots.Num(), Slots.Num());
			return;
		}

		// 빈 슬롯을 뒤에 채움
		SortedSlots.SetNum(Slots.Num());
	}

	Slots = MoveTemp(SortedSlots);

	OnInventoryUpdated_Internal();
}

bool UDC1InventoryComponentBase::IsValuableItem(FName InItemID)
{
	return DC1ItemHelper::IsValuableItem(InItemID);
}

int32 UDC1InventoryComponentBase::GetStackLimit(const FDC1ItemRow& InItemRow) const
{
	return InItemRow.MaxStack;
}

void UDC1InventoryComponentBase::OnInventoryUpdated_Internal()
{
	// 이벤트 브로드캐스트
	OnInventoryUpdated.Broadcast();

	// 추가 처리는 자식 클래스에서 오버라이드
}

void UDC1InventoryComponentBase::OnRep_Slots(const TArray<FDC1InventorySlotInfo>& OldSlots)
{
	// 실제로 변경된 슬롯만 찾아 개별 갱신 이벤트 발생 (서버의 부분 갱신 최적화와 동일한 방식)
	const int32 NumSlots = FMath::Max(Slots.Num(), OldSlots.Num());
	for (int32 i = 0; i < NumSlots; ++i)
	{
		const bool bOldValid = OldSlots.IsValidIndex(i);
		const bool bNewValid = Slots.IsValidIndex(i);

		if (!bOldValid || !bNewValid || !(OldSlots[i] == Slots[i]))
		{
			OnSlotChanged.Broadcast(i);
		}
	}

	OnInventoryUpdated.Broadcast();
}

bool UDC1InventoryComponentBase::AddItemToEmptySlot(FName InItemID, int32 InCount)
{
	// 빈 슬롯 찾기
	int32 EmptySlotIndex = FindEmptySlot();

	if (EmptySlotIndex == -1)
	{
		// 빈 슬롯이 없음
		return false;
	}

	// 슬롯에 아이템 추가
	Slots[EmptySlotIndex].ItemID = InItemID;
	Slots[EmptySlotIndex].StackCount = InCount;

	return true;
}
