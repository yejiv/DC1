// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Component/Inventory/DC1InventoryTypes.h"
#include "DC1InventoryComponentBase.generated.h"

// 인벤토리 변경 이벤트
DECLARE_MULTICAST_DELEGATE(FOnInventoryUpdated);

// 특정 슬롯 변경 이벤트 (부분 갱신용)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotChanged, int32);

/**
 * 인벤토리의 기본 기능을 제공하는 부모 컴포넌트
 * 플레이어, 상자 등 모든 인벤토리 액터에 부착 가능
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DC1_API UDC1InventoryComponentBase : public UActorComponent
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

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearSlot(int32 InSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetCapacityAndColumns(int32 InTotalCapacity, int32 InColumns);


public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(FName InItemID, int32 InCount = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(int32 InSlotIndex, int32 InCount = 1);

protected:
	virtual void InitializeSlots();
	virtual void OnInventoryUpdated_Internal();

	/** 클라이언트에서 Slots 복제 수신 시 호출되어 UI 갱신 이벤트를 발생시킴 */
	UFUNCTION()
	void OnRep_Slots(const TArray<FDC1InventorySlotInfo>& OldSlots);

private:
	bool AddItemToEmptySlot(FName InItemID, int32 InCount);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Slots, BlueprintReadOnly, Category = "Inventory")
	TArray<FDC1InventorySlotInfo> Slots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings", meta = (ClampMin = "1"))
	int32 TotalCapacity = 42;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings", meta = (ClampMin = "1"))
	int32 Columns = 6;

	// 행 개수 읽기 전용
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 Rows = 7;
};
