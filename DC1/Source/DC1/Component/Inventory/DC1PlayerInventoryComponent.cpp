// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Inventory/DC1PlayerInventoryComponent.h"
#include "Component/Inventory/DC1ChestInventoryComponent.h"
#include "Component/Inventory/DC1ValuableInventoryComponent.h"
#include "Character/Player/DC1PlayerState.h"
#include "Net/UnrealNetwork.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/UI/DC1UIData.h"
#include "Data/DA/DataTable/DC1DataTableData.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Tags/UI/DC1GameplayTag_UI.h"
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
		InitializeDefaultItems();
	}
}

void UDC1PlayerInventoryComponent::InitializeDefaultItems()
{
	if (GetOwnerRole() != ROLE_Authority)
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
	ItemDataTable->GetAllRows<FDC1ItemRow>(TEXT("Items"), AllRows);
	if (true == AllRows.IsEmpty())
	{
		return;
	}

	TMap<int32, TArray<FDC1ItemRow*>> RowsByLevel;

	for (FDC1ItemRow* Row : AllRows)
	{
		FName ItemID = Row->ItemID;
		AddItem(ItemID, 1);

		FText ItemName = Row->ItemName;
	}

	// 행 이름이 아니라 ItemID 필드 기준 (행 이름은 ItemID와 다를 수 있음)
	if (AllRows.Num() > 1 && nullptr != AllRows[1])
	{
		AddItem(AllRows[1]->ItemID, 1);
	}
}

void UDC1PlayerInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 플레이어 전용 데이터 - COND_OwnerOnly (플레이어 본인과 서버만)
	DOREPLIFETIME_CONDITION(UDC1PlayerInventoryComponent, Slots, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UDC1PlayerInventoryComponent, CurrentWeight, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UDC1PlayerInventoryComponent, EquipmentSlots, COND_OwnerOnly);
}


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

	int32 SlotIndex;
	// 이미 아이템이 있는지 확인
	if (FindItemInSlot(InItemID, &SlotIndex))
	{
		// 소유 개수 추후 추가하기
		return true;
	}

	// 빈 슬롯 또는 이미 아이템이 있는지 확인
	if (FindEmptySlot() == -1)
	{
		return false;
	}

	//// 무게는 상관 없나?
	//float EstimatedWeight = CurrentWeight + (InCount * ItemWeightPerUnit);
	//if (EstimatedWeight > MaxWeight)
	//{
	//	return false;
	//}

	return true;
}

// ========================
// 장비 슬롯 관련 함수 구현
// ========================

const FDC1InventorySlotInfo& UDC1PlayerInventoryComponent::GetEquipmentSlot(int32 InSlotIndex) const
{
	static const FDC1InventorySlotInfo EmptySlot = FDC1InventorySlotInfo();

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

	FDC1InventorySlotInfo& InvSlot = Slots[InSlotIndex];
	FDC1InventorySlotInfo& EquipSlot = EquipmentSlots[InEquipSlotIndex];

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

	FDC1InventorySlotInfo& EquipSlot = EquipmentSlots[InEquipSlotIndex];

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
// 보호 함수 구현
// ========================

void UDC1PlayerInventoryComponent::CalculateWeight()
{
	CurrentWeight = 0.0f;

	// 인벤토리 슬롯의 무게 계산
	for (const FDC1InventorySlotInfo& Slot : Slots)
	{
		if (!Slot.IsEmpty())
		{
			CurrentWeight += Slot.StackCount * ItemWeightPerUnit;
		}
	}

	// 장비 슬롯의 무게 계산
	for (const FDC1InventorySlotInfo& Slot : EquipmentSlots)
	{
		if (!Slot.IsEmpty())
		{
			CurrentWeight += Slot.StackCount * ItemWeightPerUnit;
		}
	}

	// 최대 무게 초과 방지
	CurrentWeight = FMath::Min(CurrentWeight, MaxWeight);
}

void UDC1PlayerInventoryComponent::OnRep_EquipmentSlots()
{
	OnInventoryUpdated.Broadcast();
}

void UDC1PlayerInventoryComponent::InitializeEquipmentSlots()
{
	EquipmentSlots.Empty();

	// 기본 4개 장비 슬롯 생성 (머리, 몸통, 다리, 발 등)
	for (int32 i = 0; i < 4; ++i)
	{
		EquipmentSlots.Add(FDC1InventorySlotInfo());
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

	// 부모 이벤트 브로드캐스트
	Super::OnInventoryUpdated_Internal();
}

// ========================
// 드래그 드롭 아이템 이동
// ========================

void UDC1PlayerInventoryComponent::RequestMoveItem(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount /* = -1 */)
{
	Server_MoveItem(FromSlotIndex, ToSlotIndex, MoveCount);
}

bool UDC1PlayerInventoryComponent::Server_MoveItem_Validate(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount)
{
	if (!Slots.IsValidIndex(FromSlotIndex) || !Slots.IsValidIndex(ToSlotIndex))
	{
		return false;
	}

	if (MoveCount < -1)
	{
		return false;
	}

	return true;
}

void UDC1PlayerInventoryComponent::Server_MoveItem_Implementation(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount)
{
	MoveOrMergeSlot(FromSlotIndex, ToSlotIndex, MoveCount);
}

// ========================
// 상자 연동 (드래그 앤 드롭)
// ========================

void UDC1PlayerInventoryComponent::RequestMoveChestItem(UDC1ChestInventoryComponent* InChestComp, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount /* = -1 */)
{
	Server_MoveChestItem(InChestComp, FromSlotIndex, ToSlotIndex, MoveCount);
}

bool UDC1PlayerInventoryComponent::Server_MoveChestItem_Validate(UDC1ChestInventoryComponent* InChestComp, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount)
{
	if (!IsValid(InChestComp))
	{
		return false;
	}

	if (MoveCount < -1)
	{
		return false;
	}

	return true;
}

void UDC1PlayerInventoryComponent::Server_MoveChestItem_Implementation(UDC1ChestInventoryComponent* InChestComp, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount)
{
	if (!IsValid(InChestComp))
	{
		return;
	}

	InChestComp->MoveOrMergeSlot(FromSlotIndex, ToSlotIndex, MoveCount);
}

void UDC1PlayerInventoryComponent::RequestTransferItem(UDC1ChestInventoryComponent* InChestComp, bool bToChest, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount /* = -1 */)
{
	Server_TransferItem(InChestComp, bToChest, FromSlotIndex, ToSlotIndex, MoveCount);
}

bool UDC1PlayerInventoryComponent::Server_TransferItem_Validate(UDC1ChestInventoryComponent* InChestComp, bool bToChest, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount)
{
	if (!IsValid(InChestComp))
	{
		return false;
	}

	if (FromSlotIndex < 0 || ToSlotIndex < 0)
	{
		return false;
	}

	if (MoveCount < -1)
	{
		return false;
	}

	return true;
}

void UDC1PlayerInventoryComponent::Server_TransferItem_Implementation(UDC1ChestInventoryComponent* InChestComp, bool bToChest, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount)
{
	if (!IsValid(InChestComp))
	{
		return;
	}

	if (bToChest)
	{
		// 꺼내기 전용 상자에는 넣을 수 없음
		if (false == InChestComp->CanStoreItems())
		{
			return;
		}

		TransferSlotTo(InChestComp, FromSlotIndex, ToSlotIndex, MoveCount);
	}
	else
	{
		// 꺼내기 전용 상자라면 교환으로 플레이어 아이템이 상자에 들어가는 것도 막음
		InChestComp->TransferSlotTo(this, FromSlotIndex, ToSlotIndex, MoveCount, InChestComp->CanStoreItems());
	}
}

// ========================
// 분류 버튼 (일회성 정렬)
// ========================

void UDC1PlayerInventoryComponent::RequestSortInventory()
{
	Server_SortInventory();
}

bool UDC1PlayerInventoryComponent::Server_SortInventory_Validate()
{
	return true;
}

void UDC1PlayerInventoryComponent::Server_SortInventory_Implementation()
{
	SortSlots();
}

void UDC1PlayerInventoryComponent::RequestSortChestItems(UDC1ChestInventoryComponent* InChestComp)
{
	Server_SortChestItems(InChestComp);
}

bool UDC1PlayerInventoryComponent::Server_SortChestItems_Validate(UDC1ChestInventoryComponent* InChestComp)
{
	return IsValid(InChestComp);
}

void UDC1PlayerInventoryComponent::Server_SortChestItems_Implementation(UDC1ChestInventoryComponent* InChestComp)
{
	if (false == ::IsValid(InChestComp))
	{
		return;
	}

	InChestComp->SortSlots();
}

void UDC1PlayerInventoryComponent::RequestSortValuables()
{
	Server_SortValuables();
}

bool UDC1PlayerInventoryComponent::Server_SortValuables_Validate()
{
	return true;
}

void UDC1PlayerInventoryComponent::Server_SortValuables_Implementation()
{
	UDC1ValuableInventoryComponent* ValuableInventory = FindValuableInventory();
	if (false == ::IsValid(ValuableInventory))
	{
		return;
	}

	ValuableInventory->SortSlots();
}

// ========================
// 귀중품 분기
// ========================

bool UDC1PlayerInventoryComponent::AddItem(FName InItemID, int32 InCount /* = 1 */)
{
	// 귀중품은 귀중품 인벤토리로 넘김 (서버 권한 검사는 넘겨받은 쪽에서 수행)
	if (true == IsValuableItem(InItemID))
	{
		UDC1ValuableInventoryComponent* ValuableInventory = FindValuableInventory();
		return ::IsValid(ValuableInventory) && ValuableInventory->AddItem(InItemID, InCount);
	}

	return Super::AddItem(InItemID, InCount);
}

bool UDC1PlayerInventoryComponent::CanAcceptItem(FName InItemID) const
{
	return false == IsValuableItem(InItemID);
}

UDC1ValuableInventoryComponent* UDC1PlayerInventoryComponent::FindValuableInventory() const
{
	const ADC1PlayerState* OwnerPlayerState = Cast<ADC1PlayerState>(GetOwner());
	if (false == ::IsValid(OwnerPlayerState))
	{
		return nullptr;
	}

	return OwnerPlayerState->GetValuableInventoryComponent();
}
