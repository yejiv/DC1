// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Inventory/Player/DC1PlayerInventoryComponent.h"
#include "Component/Inventory/Chest/DC1ChestInventoryComponent.h"
#include "Component/Inventory/Valuable/DC1ValuableInventoryComponent.h"
#include "Character/Player/DC1PlayerState.h"
#include "Net/UnrealNetwork.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/UI/DC1UIData.h"
#include "Data/DA/DataTable/DC1DataTableData.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1ItemHelper.h"
#include "Define/DC1Macro.h"
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
		InitializeDefaultEquipment();
		InitializeDefaultItems();

		// 먹이 주머니가 귀중품 인벤토리에 있을 수 있으므로 함께 감시
		if (UDC1ValuableInventoryComponent* ValuableInventory = FindValuableInventory())
		{
			ValuableInventory->OnInventoryUpdated.AddUObject(this, &ThisClass::HandleValuableInventoryUpdated);
		}

		RefreshFoodSlotCount();
	}
}

void UDC1PlayerInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UDC1ValuableInventoryComponent* ValuableInventory = FindValuableInventory())
	{
		ValuableInventory->OnInventoryUpdated.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UDC1PlayerInventoryComponent::InitializeDefaultEquipment()
{
	int32 WeaponIndex = 0;
	for (const FName& ItemID : DefaultWeaponItemIDs)
	{
		const int32 EquipSlotIndex = DC1EquipSlotLayout::GetEquipIndex(EDC1EquipSlotType::Weapon, WeaponIndex);
		if (-1 == EquipSlotIndex)
		{
			break;
		}

		// 장비 테이블에 없거나 무기가 아니면 건너뜀
		if (false == CanEquipToSlot(ItemID, EquipSlotIndex))
		{
			LOG_WARNING(TEXT("[Inventory] 시작 무기 %s 장착 불가 (장비 테이블 확인)"), *ItemID.ToString());
			continue;
		}

		EquipmentSlots[EquipSlotIndex].ItemID = ItemID;
		EquipmentSlots[EquipSlotIndex].StackCount = 1;
		HandleEquipmentChanged(EquipSlotIndex);

		++WeaponIndex;
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
	DOREPLIFETIME_CONDITION(UDC1PlayerInventoryComponent, UnlockedWeaponCount, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UDC1PlayerInventoryComponent, UnlockedAccessoryCount, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UDC1PlayerInventoryComponent, UnlockedFoodCount, COND_OwnerOnly);
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

bool UDC1PlayerInventoryComponent::CanEquipToSlot(FName InItemID, int32 InEquipSlotIndex) const
{
	const EDC1EquipSlotType SlotType = DC1EquipSlotLayout::GetSlotType(InEquipSlotIndex);
	if (EDC1EquipSlotType::None == SlotType)
	{
		return false;
	}

	if (false == IsEquipSlotUnlocked(InEquipSlotIndex))
	{
		return false;
	}

	// 장비 테이블에 행이 없으면 장비 불가 아이템
	const FDC1EquipItemRow* EquipRow = DC1ItemHelper::FindEquipItemRow(InItemID);
	if (nullptr == EquipRow)
	{
		return false;
	}

	return SlotType == EquipRow->EquipSlotType;
}

int32 UDC1PlayerInventoryComponent::GetUnlockedSlotCount(EDC1EquipSlotType InSlotType) const
{
	switch (InSlotType)
	{
	case EDC1EquipSlotType::Weapon:
		return UnlockedWeaponCount;
	case EDC1EquipSlotType::Accessory:
		return UnlockedAccessoryCount;
	case EDC1EquipSlotType::Food:
		return UnlockedFoodCount;
	default:
		return DC1EquipSlotLayout::GetTypeSlotCount(InSlotType);
	}
}

bool UDC1PlayerInventoryComponent::IsEquipSlotUnlocked(int32 InEquipSlotIndex) const
{
	const EDC1EquipSlotType SlotType = DC1EquipSlotLayout::GetSlotType(InEquipSlotIndex);
	if (EDC1EquipSlotType::None == SlotType)
	{
		return false;
	}

	// 같은 타입 칸 중 앞에서부터 열린 칸 수만큼 열림
	return DC1EquipSlotLayout::GetLocalIndex(InEquipSlotIndex) < GetUnlockedSlotCount(SlotType);
}

void UDC1PlayerInventoryComponent::SetUnlockedSlotCount(EDC1EquipSlotType InSlotType, int32 InCount)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	const int32 NewCount = FMath::Clamp(InCount, 0, DC1EquipSlotLayout::GetTypeSlotCount(InSlotType));

	switch (InSlotType)
	{
	case EDC1EquipSlotType::Weapon:
		UnlockedWeaponCount = NewCount;
		break;
	case EDC1EquipSlotType::Accessory:
		UnlockedAccessoryCount = NewCount;
		break;
	default:
		// 음식은 먹이 주머니로만, 나머지 타입은 항상 열림
		return;
	}

	OnEquipmentUpdated.Broadcast();
}

void UDC1PlayerInventoryComponent::RefreshFoodSlotCount()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	int32 MaxPouchSlotCount = 0;

	const auto CheckSlots = [&MaxPouchSlotCount](const TArray<FDC1InventorySlotInfo>& InSlots)
	{
		for (const FDC1InventorySlotInfo& Slot : InSlots)
		{
			if (true == Slot.IsEmpty())
			{
				continue;
			}

			if (const FDC1ItemRow* ItemRow = DC1ItemHelper::FindItemRow(Slot.ItemID))
			{
				MaxPouchSlotCount = FMath::Max(MaxPouchSlotCount, ItemRow->FoodPouchSlotCount);
			}
		}
	};

	CheckSlots(Slots);

	if (const UDC1ValuableInventoryComponent* ValuableInventory = FindValuableInventory())
	{
		CheckSlots(ValuableInventory->GetSlots());
	}

	const int32 NewCount = FMath::Clamp(MaxPouchSlotCount, 0, DC1EquipSlotLayout::GetTypeSlotCount(EDC1EquipSlotType::Food));
	if (NewCount == UnlockedFoodCount)
	{
		return;
	}

	UnlockedFoodCount = NewCount;
	OnEquipmentUpdated.Broadcast();
}

void UDC1PlayerInventoryComponent::HandleValuableInventoryUpdated()
{
	RefreshFoodSlotCount();
}

void UDC1PlayerInventoryComponent::OnRep_UnlockedSlotCounts()
{
	OnEquipmentUpdated.Broadcast();
}

void UDC1PlayerInventoryComponent::OnRep_CurrentWeight()
{
	OnInventoryUpdated.Broadcast();
}

bool UDC1PlayerInventoryComponent::DropItem(int32 InSlotIndex, int32 InCount)
{
	// TODO : 월드에 아이템 스폰 (픽업 시스템이 생기면 연결)
	return RemoveItem(InSlotIndex, InCount);
}

bool UDC1PlayerInventoryComponent::DiscardItem(int32 InSlotIndex, int32 InCount)
{
	return RemoveItem(InSlotIndex, InCount);
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

	// 장비 칸 타입이 맞지 않으면 실패
	if (false == CanEquipToSlot(InvSlot.ItemID, InEquipSlotIndex))
	{
		return false;
	}

	// 칸이 차 있으면 교환
	if (!EquipSlot.IsEmpty())
	{
		Swap(InvSlot, EquipSlot);
	}
	else
	{
		// 장비 슬롯이 비어있으면 이동 (스택 전체)
		EquipSlot = InvSlot;
		InvSlot.Clear();
	}

	OnSlotChanged.Broadcast(InSlotIndex);
	HandleEquipmentChanged(InEquipSlotIndex);

	return true;
}

bool UDC1PlayerInventoryComponent::EquipItemAuto(int32 InSlotIndex)
{
	// 서버에서만 실행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	if (!Slots.IsValidIndex(InSlotIndex) || Slots[InSlotIndex].IsEmpty())
	{
		return false;
	}

	const int32 EquipSlotIndex = FindEquipSlotForItem(Slots[InSlotIndex].ItemID);
	if (-1 == EquipSlotIndex)
	{
		return false;
	}

	return EquipItem(InSlotIndex, EquipSlotIndex);
}

bool UDC1PlayerInventoryComponent::UnequipItem(int32 InEquipSlotIndex, int32 InToSlotIndex /* = -1 */)
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

	// 목표 슬롯 결정 (-1이면 첫 빈 칸)
	const int32 ToSlotIndex = (-1 == InToSlotIndex) ? FindEmptySlot() : InToSlotIndex;
	if (!Slots.IsValidIndex(ToSlotIndex))
	{
		return false;
	}

	FDC1InventorySlotInfo& InvSlot = Slots[ToSlotIndex];

	if (InvSlot.IsEmpty())
	{
		// 인벤토리로 이동
		InvSlot = EquipSlot;
		EquipSlot.Clear();
	}
	else if (true == CanEquipToSlot(InvSlot.ItemID, InEquipSlotIndex))
	{
		// 목표 슬롯 아이템이 이 장비 칸에 맞으면 교환
		Swap(InvSlot, EquipSlot);
	}
	else
	{
		return false;
	}

	OnSlotChanged.Broadcast(ToSlotIndex);
	HandleEquipmentChanged(InEquipSlotIndex);
	return true;
}

int32 UDC1PlayerInventoryComponent::FindEquipSlotForItem(FName InItemID) const
{
	const FDC1EquipItemRow* EquipRow = DC1ItemHelper::FindEquipItemRow(InItemID);
	if (nullptr == EquipRow || EDC1EquipSlotType::None == EquipRow->EquipSlotType)
	{
		return -1;
	}

	int32 FirstMatchIndex = -1;
	for (int32 Index = 0; Index < EquipmentSlots.Num(); ++Index)
	{
		if (EquipRow->EquipSlotType != DC1EquipSlotLayout::GetSlotType(Index) || false == IsEquipSlotUnlocked(Index))
		{
			continue;
		}

		// 빈 칸 우선
		if (true == EquipmentSlots[Index].IsEmpty())
		{
			return Index;
		}

		if (-1 == FirstMatchIndex)
		{
			FirstMatchIndex = Index;
		}
	}

	// 빈 칸이 없으면 그 타입의 첫 칸과 교환
	return FirstMatchIndex;
}

void UDC1PlayerInventoryComponent::HandleEquipmentChanged(int32 InEquipSlotIndex)
{
	// 무게 재계산 + 인벤토리 갱신 이벤트
	OnInventoryUpdated_Internal();

	// TODO : 방어구/장신구/음식 효과는 구독 측(EquipComponent)에서 처리
	OnEquipmentSlotChanged.Broadcast(InEquipSlotIndex);
	OnEquipmentUpdated.Broadcast();
}

// ========================
// 보호 함수 구현
// ========================

void UDC1PlayerInventoryComponent::CalculateWeight()
{
	CurrentWeight = 0.0f;

	// 아이템 Weight x 개수 (인벤토리 + 장비 칸)
	const auto AddSlotsWeight = [this](const TArray<FDC1InventorySlotInfo>& InSlots)
	{
		for (const FDC1InventorySlotInfo& Slot : InSlots)
		{
			if (true == Slot.IsEmpty())
			{
				continue;
			}

			if (const FDC1ItemRow* ItemRow = DC1ItemHelper::FindItemRow(Slot.ItemID))
			{
				CurrentWeight += Slot.StackCount * ItemRow->Weight;
			}
		}
	};

	AddSlotsWeight(Slots);
	AddSlotsWeight(EquipmentSlots);

	// 최대 무게 초과 방지
	CurrentWeight = FMath::Min(CurrentWeight, MaxWeight);
}

void UDC1PlayerInventoryComponent::OnRep_EquipmentSlots()
{
	OnInventoryUpdated.Broadcast();
	OnEquipmentUpdated.Broadcast();
}

void UDC1PlayerInventoryComponent::InitializeEquipmentSlots()
{
	// DC1EquipSlotLayout 칸 수만큼 생성
	EquipmentSlots.Empty();
	EquipmentSlots.SetNum(DC1EquipSlotLayout::SlotCount);
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

	// 먹이 주머니를 얻었을 수 있으므로 음식 칸 수 갱신
	RefreshFoodSlotCount();

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
// 장비 착용 / 해제 요청
// ========================

void UDC1PlayerInventoryComponent::RequestEquipItem(int32 InSlotIndex, int32 InEquipSlotIndex)
{
	Server_EquipItem(InSlotIndex, InEquipSlotIndex);
}

bool UDC1PlayerInventoryComponent::Server_EquipItem_Validate(int32 InSlotIndex, int32 InEquipSlotIndex)
{
	// 범위 밖 인덱스만 거름 (false면 클라 킥, 상태 실패는 Implementation에서 무시)
	return 0 <= InSlotIndex && 0 <= InEquipSlotIndex && InEquipSlotIndex < DC1EquipSlotLayout::SlotCount;
}

void UDC1PlayerInventoryComponent::Server_EquipItem_Implementation(int32 InSlotIndex, int32 InEquipSlotIndex)
{
	EquipItem(InSlotIndex, InEquipSlotIndex);
}

void UDC1PlayerInventoryComponent::RequestEquipItemAuto(int32 InSlotIndex)
{
	Server_EquipItemAuto(InSlotIndex);
}

bool UDC1PlayerInventoryComponent::Server_EquipItemAuto_Validate(int32 InSlotIndex)
{
	return 0 <= InSlotIndex;
}

void UDC1PlayerInventoryComponent::Server_EquipItemAuto_Implementation(int32 InSlotIndex)
{
	EquipItemAuto(InSlotIndex);
}

void UDC1PlayerInventoryComponent::RequestUnequipItem(int32 InEquipSlotIndex, int32 InToSlotIndex /* = -1 */)
{
	Server_UnequipItem(InEquipSlotIndex, InToSlotIndex);
}

bool UDC1PlayerInventoryComponent::Server_UnequipItem_Validate(int32 InEquipSlotIndex, int32 InToSlotIndex)
{
	return 0 <= InEquipSlotIndex && InEquipSlotIndex < DC1EquipSlotLayout::SlotCount && -1 <= InToSlotIndex;
}

void UDC1PlayerInventoryComponent::Server_UnequipItem_Implementation(int32 InEquipSlotIndex, int32 InToSlotIndex)
{
	UnequipItem(InEquipSlotIndex, InToSlotIndex);
}

// ========================
// 버리기 / 폐기 요청
// ========================

void UDC1PlayerInventoryComponent::RequestDropItem(int32 InSlotIndex, int32 InCount)
{
	Server_DropItem(InSlotIndex, InCount);
}

bool UDC1PlayerInventoryComponent::Server_DropItem_Validate(int32 InSlotIndex, int32 InCount)
{
	return 0 <= InSlotIndex && 0 < InCount;
}

void UDC1PlayerInventoryComponent::Server_DropItem_Implementation(int32 InSlotIndex, int32 InCount)
{
	DropItem(InSlotIndex, InCount);
}

void UDC1PlayerInventoryComponent::RequestDiscardItem(int32 InSlotIndex, int32 InCount)
{
	Server_DiscardItem(InSlotIndex, InCount);
}

bool UDC1PlayerInventoryComponent::Server_DiscardItem_Validate(int32 InSlotIndex, int32 InCount)
{
	return 0 <= InSlotIndex && 0 < InCount;
}

void UDC1PlayerInventoryComponent::Server_DiscardItem_Implementation(int32 InSlotIndex, int32 InCount)
{
	DiscardItem(InSlotIndex, InCount);
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
