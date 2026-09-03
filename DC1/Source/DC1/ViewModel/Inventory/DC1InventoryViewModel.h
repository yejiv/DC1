// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Component/Inventory/DC1InventoryComponentBase.h"
#include "Component/Inventory/DC1InventoryTypes.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "DC1InventoryViewModel.generated.h"

// Forward Declarations
class UDC1PlayerInventoryComponent;
class UDC1ChestInventoryComponent;

// 특정 슬롯 갱신 이벤트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotUIUpdated, int32);

// 드래그 세션 종료 이벤트 (드래그를 시작한 위젯이 시각 복제본을 정리할 수 있도록 알림)
DECLARE_MULTICAST_DELEGATE(FOnDragEnded);

UCLASS()
class DC1_API UDC1InventoryViewModel : public UObject
{
	GENERATED_BODY()

public:
	UDC1InventoryViewModel();

public:
	/** 전체 인벤토리 갱신 이벤트 (레거시, 사용 권장 안함) */
	FOnInventoryUpdated OnInventoryUpdated;

	/** 특정 슬롯만 갱신하는 이벤트 (성능 최적화) */
	FOnSlotUIUpdated OnSlotUIUpdated;

	/** 상자 전용: 전체 갱신 이벤트 */
	FOnInventoryUpdated OnChestInventoryUpdated;

	/** 상자 전용: 특정 슬롯만 갱신하는 이벤트 */
	FOnSlotUIUpdated OnChestSlotUIUpdated;

	/** 드래그 세션 종료 이벤트 */
	FOnDragEnded OnDragEnded;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetPlayerInventoryComponent(UDC1PlayerInventoryComponent* InPlayerComp);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetTargetChest(UDC1ChestInventoryComponent* InChestComp);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	UDC1PlayerInventoryComponent* GetPlayerInventoryComponent() const { return MyInventoryComp; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	UDC1ChestInventoryComponent* GetTargetChest() const { return TargetChestComp; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool IsChestOpen() const { return TargetChestComp != nullptr; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	const TArray<FDC1InventorySlotInfo>& GetInventorySlots() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetSlotCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	const FDC1InventorySlotInfo& GetSlot(int32 Index) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	const TArray<FDC1InventorySlotInfo>& GetChestInventorySlots() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetChestSlotCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	const FDC1InventorySlotInfo& GetChestSlot(int32 Index) const;

	const FDC1ItemRow* GetItemData(FName ItemID) const;

	/**
	 * 특정 슬롯을 갱신
	 * @param SlotIndex 갱신할 슬롯 인덱스
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshSlot(int32 SlotIndex);

	/**
	 * 인벤토리 아이템 이동 요청
	 * @param FromSlotIndex 출발 슬롯
	 * @param ToSlotIndex 목표 슬롯
	 * @param MoveCount 이동할 개수 (기본값: -1, 전체 이동)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestMoveItem(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount = -1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestMoveChestItem(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount = -1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestTransferItem(bool bToChest, int32 FromSlotIndex, int32 MoveCount = -1);

public:
	// ========================
	// 드래그 세션 (플레이어 위젯 / 상자 위젯이 동일 인스턴스를 공유하여 서로의 드래그를 인지)
	// ========================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Drag")
	bool IsDragging() const { return bIsDragging; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Drag")
	bool IsDragSourceChest() const { return bDragSourceIsChest; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Drag")
	int32 GetDragSourceSlotIndex() const { return DragSourceSlotIndex; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Drag")
	int32 GetDragStackCount() const { return DragStackCount; }

	const FDC1ItemRow* GetDragItemData() const { return DragItemData; }

	/**
	 * 드래그 시작
	 * @param bFromChest 상자 슬롯에서 시작한 드래그인지 여부
	 * @param SlotIndex 드래그 시작 슬롯 (bFromChest 기준 인덱스)
	 * @param StackCount 드래그로 옮길 개수
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Drag")
	void BeginDrag(bool bFromChest, int32 SlotIndex, int32 StackCount);

	/** 드래그 세션 종료 (드롭 완료/취소 공용) */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Drag")
	void EndDrag();

	/**
	 * 드래그 중이던 아이템을 대상 슬롯에 드롭 요청
	 * 소스/타겟 조합(플레이어-플레이어, 상자-상자, 플레이어-상자)에 맞는 이동 함수로 자동 라우팅 후 드래그 세션 종료
	 * @param bTargetIsChest 드롭 대상이 상자 슬롯인지 여부
	 * @param TargetSlotIndex 드롭 대상 슬롯 (bTargetIsChest 기준 인덱스)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Drag")
	void RequestDrop(bool bTargetIsChest, int32 TargetSlotIndex);

protected:
	void HandleInventoryUpdated();
	void HandleSlotUpdated(int32 SlotIndex);
	void HandleChestInventoryUpdated();
	void HandleChestSlotUpdated(int32 SlotIndex);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UDC1PlayerInventoryComponent> MyInventoryComp;

	/** 현재 열린 상자 컴포넌트 (nullptr이면 없음) */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UDC1ChestInventoryComponent> TargetChestComp;

private:
	bool bIsDragging = false;
	bool bDragSourceIsChest = false;
	int32 DragSourceSlotIndex = -1;
	int32 DragStackCount = 0;
	const FDC1ItemRow* DragItemData = nullptr;
};
