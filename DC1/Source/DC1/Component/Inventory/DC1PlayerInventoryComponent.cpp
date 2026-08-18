// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Inventory/DC1PlayerInventoryComponent.h"
#include "Net/UnrealNetwork.h"

UDC1PlayerInventoryComponent::UDC1PlayerInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// 플레이어 기본값
	TotalCapacity = 42;
	Columns = 6;
	MaxWeight = 100.0f;
}

void UDC1PlayerInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		InitializeEquipmentSlots();
	}
}

void UDC1PlayerInventoryComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 플레이어 전용 데이터 - COND_OwnerOnly (플레이어 본인과 서버만)
	DOREPLIFETIME_CONDITION(UDC1PlayerInventoryComponent, CurrentWeight, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UDC1PlayerInventoryComponent, EquipmentSlots, COND_OwnerOnly);
}

// ========================
// 무게 관련 함수 구현
// ========================

float UDC1PlayerInventoryComponent::GetWeightPercentage() const
{
	if (MaxWeight <= 0.0f)
	{
		return 1.0f;
	}

	return FMath::Clamp(CurrentWeight / MaxWeight, 0.0f, 1.0f);
}

float UDC1PlayerInventoryComponent::GetRemainingWeight() const
{
	return FMath::Max(0.0f, MaxWeight - CurrentWeight);
}

bool UDC1PlayerInventoryComponent::CanAddItem(FName InItemID, int32 InCount) const
{
	if (InItemID.IsNone() || InCount <= 0)
	{
		return false;
	}

	// 빈 슬롯이 있는지 확인
	if (FindEmptySlot() == -1)
	{
		return false;
	}

	// 무게 체크
	float EstimatedWeight = CurrentWeight + (InCount * ItemWeightPerUnit);
	if (EstimatedWeight > MaxWeight)
	{
		return false;
	}

	return true;
}

// ========================
// 장비 슬롯 관련 함수 구현
// ========================

const FDC1InventorySlot& UDC1PlayerInventoryComponent::GetEquipmentSlot(int32 InSlotIndex) const
{
	static const FDC1InventorySlot EmptySlot = FDC1InventorySlot();

	if (!EquipmentSlots.IsValidIndex(InSlotIndex))
	{
		return EmptySlot;
	}

	return EquipmentSlots[InSlotIndex];
}

bool UDC1PlayerInventoryComponent::EquipItem(int32 InSlotIndex, int32 InEquipSlotIndex)
{
	// 서버에서만 실행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	// 유효성 검사
	if (!Slots.IsValidIndex(InSlotIndex) || !EquipmentSlots.IsValidIndex(InEquipSlotIndex))
	{
		return false;
	}

	FDC1InventorySlot& InvSlot = Slots[InSlotIndex];
	FDC1InventorySlot& EquipSlot = EquipmentSlots[InEquipSlotIndex];

	// 인벤토리 슬롯이 비어있으면 실패
	if (InvSlot.IsEmpty())
	{
		return false;
	}

	// 장비 슬롯에 이미 아이템이 있으면 교환
	if (!EquipSlot.IsEmpty())
	{
		Swap(InvSlot, EquipSlot);
	}
	else
	{
		// 장비 슬롯이 비어있으면 이동
		EquipSlot = InvSlot;
		InvSlot.Clear();
	}

	OnInventoryUpdated_Internal();
	return true;
}

bool UDC1PlayerInventoryComponent::UnequipItem(int32 InEquipSlotIndex)
{
	// 서버에서만 실행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	// 유효성 검사
	if (!EquipmentSlots.IsValidIndex(InEquipSlotIndex))
	{
		return false;
	}

	FDC1InventorySlot& EquipSlot = EquipmentSlots[InEquipSlotIndex];

	// 장비 슬롯이 비어있으면 실패
	if (EquipSlot.IsEmpty())
	{
		return false;
	}

	// 빈 인벤토리 슬롯 찾기
	int32 EmptySlotIndex = FindEmptySlot();
	if (EmptySlotIndex == -1)
	{
		return false;
	}

	// 인벤토리로 이동
	Slots[EmptySlotIndex] = EquipSlot;
	EquipSlot.Clear();

	OnInventoryUpdated_Internal();
	return true;
}

// ========================
// 정렬 관련 함수 구현
// ========================

void UDC1PlayerInventoryComponent::SortInventory()
{
	// 서버에서만 실행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	// 간단한 정렬: 아이템을 앞쪽에 모음
	TArray<FDC1InventorySlot> SortedSlots;
	SortedSlots.Reserve(Slots.Num());

	// 아이템이 있는 슬롯 먼저
	for (const FDC1InventorySlot& Slot : Slots)
	{
		if (!Slot.IsEmpty())
		{
			SortedSlots.Add(Slot);
		}
	}

	// 빈 슬롯을 뒤에 추가
	while (SortedSlots.Num() < Slots.Num())
	{
		SortedSlots.Add(FDC1InventorySlot());
	}

	Slots = SortedSlots;

	OnInventoryUpdated_Internal();
}

// ========================
// 보호 함수 구현
// ========================

void UDC1PlayerInventoryComponent::CalculateWeight()
{
	CurrentWeight = 0.0f;

	// 인벤토리 슬롯의 무게 계산
	for (const FDC1InventorySlot& Slot : Slots)
	{
		if (!Slot.IsEmpty())
		{
			CurrentWeight += Slot.StackCount * ItemWeightPerUnit;
		}
	}

	// 장비 슬롯의 무게 계산
	for (const FDC1InventorySlot& Slot : EquipmentSlots)
	{
		if (!Slot.IsEmpty())
		{
			CurrentWeight += Slot.StackCount * ItemWeightPerUnit;
		}
	}

	// 최대 무게 초과 방지
	CurrentWeight = FMath::Min(CurrentWeight, MaxWeight);
}

void UDC1PlayerInventoryComponent::InitializeEquipmentSlots()
{
	EquipmentSlots.Empty();

	// 기본 4개 장비 슬롯 생성 (머리, 몸통, 다리, 발 등)
	for (int32 i = 0; i < 4; ++i)
	{
		EquipmentSlots.Add(FDC1InventorySlot());
	}

	// UE_LOG(LogTemp, Warning, TEXT("[%s] Equipment slots initialized: 4 slots"),
	//	*GetOwner()->GetName());
}

void UDC1PlayerInventoryComponent::InitializeSlots()
{
	// 부모 클래스의 슬롯 초기화
	Super::InitializeSlots();

	// 무게 초기화
	CurrentWeight = 0.0f;
}

void UDC1PlayerInventoryComponent::OnInventoryUpdated_Internal()
{
	// 무게 재계산
	CalculateWeight();

	// 자동 정렬 활성화 시 정렬
	if (bAutoSort && GetOwnerRole() == ROLE_Authority)
	{
		SortInventory();
	}

	// 부모 이벤트 브로드캐스트
	Super::OnInventoryUpdated_Internal();
}
