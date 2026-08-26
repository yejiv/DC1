// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Inventory/DC1InventoryComponentBase.h"
#include "Net/UnrealNetwork.h"

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

void UDC1InventoryComponentBase::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Slots 배열을 복제
	DOREPLIFETIME(UDC1InventoryComponentBase, Slots);
}

// ========================
// 조회 함수 구현
// ========================

const FDC1InventorySlot& UDC1InventoryComponentBase::GetSlot(int32 InIndex) const
{
	static const FDC1InventorySlot EmptySlot = FDC1InventorySlot();

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
	for (const FDC1InventorySlot& Slot : Slots)
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

bool UDC1InventoryComponentBase::FindItemInSlot(FName InItemID, int32* SlotIndex) const
{
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i].ItemID.IsEqual(InItemID))
		{
			if(SlotIndex)
				*SlotIndex = i;

			return true;
		}
	}

	return false;
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

	int32 SlotIndex = INDEX_NONE;

	if (FindItemInSlot(InItemID, &SlotIndex))
	{
		// 기존 수량에 누적 (덮어쓰기 방지)
		Slots[SlotIndex].StackCount += InCount;
		OnInventoryUpdated_Internal();
		return true;
	}

	if (AddItemToEmptySlot(InItemID, InCount))
	{
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

	FDC1InventorySlot& Slot = Slots[InSlotIndex];

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

	OnInventoryUpdated_Internal();
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

	// 슬롯 비우기
	Slots[InSlotIndex].Clear();

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
			Slots.Add(FDC1InventorySlot());
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
		Slots.Add(FDC1InventorySlot());
	}

	// UE_LOG(LogTemp, Warning, TEXT("[%s] Inventory initialized: %d slots (%d x %d)"),
	//	*GetOwner()->GetName(), TotalCapacity, Rows, Columns);
}

void UDC1InventoryComponentBase::OnInventoryUpdated_Internal()
{
	// 이벤트 브로드캐스트
	OnInventoryUpdated.Broadcast();

	// 추가 처리는 자식 클래스에서 오버라이드
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
