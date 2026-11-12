// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Component/Inventory/DC1InventoryTypes.h"
#include "Interface/DC1SaveInterface.h"
#include "DC1InventoryComponentBase.generated.h"

struct FDC1ItemRow;

// 인벤토리 변경 이벤트
DECLARE_MULTICAST_DELEGATE(FOnInventoryUpdated);

// 특정 슬롯 변경 이벤트 (부분 갱신용)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotChanged, int32);

/**
 * 인벤토리의 기본 기능을 제공하는 부모 컴포넌트
 * 플레이어, 상자 등 모든 인벤토리 액터에 부착 가능
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DC1_API UDC1InventoryComponentBase : public UActorComponent, public IDC1SaveInterface
{
	GENERATED_BODY()

public:
	UDC1InventoryComponentBase();

public:
	/** 전체 인벤토리 변경 이벤트 (레거시) */
	FOnInventoryUpdated OnInventoryUpdated;

	/** 특정 슬롯 변경 이벤트 (부분 갱신용) */
	FOnSlotChanged OnSlotChanged;

public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 세이브 로드 완료 후 ( 서버 )
	virtual void OnPostLoad() override;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	const TArray<FDC1InventorySlotInfo>& GetSlots() const { return Slots; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	const FDC1InventorySlotInfo& GetSlot(int32 InIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetCapacity() const { return TotalCapacity; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool IsSlotEmpty(int32 InIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool IsInventoryFull() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 FindEmptySlot() const;

	bool FindItemInSlot(FName InItemID, OUT int32* SlotIndex) const;

	// 모든 슬롯 합계
	int32	GetItemCount(FName InItemID) const;
	// 슬롯별 Count 차감, 부족하면 false ( 차감 X )
	bool	RemoveItemByID(FName InItemID, int32 InCount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SwapSlots(int32 InIndex1, int32 InIndex2);

	/**
	 * 두 슬롯 사이의 아이템을 이동/병합/교환 (드래그 앤 드롭용)
	 * 같은 아이템이면 병합, 다른 아이템이면 교환, 빈 슬롯이면 이동
	 * @param FromIndex 출발 슬롯
	 * @param ToIndex 목표 슬롯
	 * @param MoveCount 이동할 개수 (기본값: -1, 전체 이동)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void MoveOrMergeSlot(int32 FromIndex, int32 ToIndex, int32 MoveCount = -1);

	/**
	 * 다른 인벤토리의 특정 슬롯으로 아이템을 이동/병합/교환 (플레이어 <-> 상자 드래그 앤 드롭용)
	 * MoveOrMergeSlot과 동일한 규칙: 빈 슬롯이면 이동, 같은 아이템이면 병합, 다른 아이템이면 교환
	 * @param InTargetComp 목표 인벤토리
	 * @param FromIndex 이 인벤토리의 출발 슬롯
	 * @param ToIndex 목표 인벤토리의 슬롯
	 * @param MoveCount 이동할 개수 (기본값: -1, 전체 이동)
	 * @param bAllowSwap false면 다른 아이템이 있는 슬롯과의 교환을 거부 (꺼내기 전용 상자에 아이템이 들어가는 것 방지)
	 * @return 슬롯 변경이 일어났는지 여부
	 */
	bool TransferSlotTo(UDC1InventoryComponentBase* InTargetComp, int32 FromIndex, int32 ToIndex, int32 MoveCount = -1, bool bAllowSwap = true);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearSlot(int32 InSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetCapacityAndColumns(int32 InTotalCapacity, int32 InColumns);

public:
	// 전부 들어갈 수 있을때만 인벤토리에 넣고 전부 못들어갈 시 넣지 않음
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool AddItem(FName InItemID, int32 InCount = 1);
	// 들어갈 수 있을만큼 넣고 남은 수 반환
	virtual int32 AddItemPartial(FName InItemID, int32 InCount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual bool RemoveItem(int32 InSlotIndex, int32 InCount = 1);

	/** 이 인벤토리에 보관할 수 있는 아이템인지 (파생 클래스가 귀중품/일반 아이템 구분 규칙을 정함) */
	virtual bool CanAcceptItem(FName InItemID) const { return true; }

	/** 귀중품 DataTable에 있는지 조회 (ItemID 필드 기준) */
	static bool IsValuableItem(FName InItemID);

	/**
	 * 슬롯 일회성 정렬 (분류 버튼용, 서버 전용)
	 * 같은 아이템 병합(MaxStack 단위로 분할) -> ItemID, 레어도 순 정렬 -> 앞쪽 슬롯부터 채움
	 * DataTable에 없는 아이템은 병합하지 않고 맨 뒤로 보냄
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SortSlots();

protected:
	virtual void InitializeSlots();
	virtual void OnInventoryUpdated_Internal();

	/** 정렬 시 한 슬롯에 담을 수 있는 최대 개수 (기본: 아이템의 MaxStack) */
	virtual int32 GetStackLimit(const FDC1ItemRow& InItemRow) const;

	/** true면 슬롯 수가 고정(빈 칸 유지), false면 가진 아이템 수만큼만 슬롯이 존재 (귀중품) */
	virtual bool IsFixedCapacity() const { return true; }

	/** 클라이언트에서 Slots 복제 수신 시 호출되어 UI 갱신 이벤트를 발생시킴 */
	UFUNCTION()
	void OnRep_Slots(const TArray<FDC1InventorySlotInfo>& OldSlots);

private:
	bool AddItemToEmptySlot(FName InItemID, int32 InCount);

	bool CanAddItemToSlots(FName InItemID) const;
	// 해당 아이템 최대 Stack
	int32 GetSlotStackLimit(FName InItemID) const;
	// 인벤토리에 넣을 수 있는 최대 개수
	int32 GetFillableCount(FName InItemID) const;
	// 기존 슬롯에 추가 후 빈칸에 추가 -> 그 이후 추가하지 못한 남은 수만큼 return
	int32 FillSlots(FName InItemID, int32 InCount);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Slots, BlueprintReadOnly, SaveGame, Category = "Inventory")
	TArray<FDC1InventorySlotInfo> Slots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings", meta = (ClampMin = "1"))
	int32 TotalCapacity = 42;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings", meta = (ClampMin = "1"))
	int32 Columns = 6;

	// 행 개수 읽기 전용
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 Rows = 7;

private:
	bool bLoadedFromSave = { false };
};
