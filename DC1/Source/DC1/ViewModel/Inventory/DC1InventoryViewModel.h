// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "Component/Inventory/DC1InventoryComponentBase.h"
#include "Component/Inventory/DC1InventoryTypes.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1Enums.h"
#include "DC1InventoryViewModel.generated.h"

// Forward Declarations
class UDC1PlayerInventoryComponent;
class UDC1ChestInventoryComponent;
class UDC1ValuableInventoryComponent;

// 특정 슬롯 갱신 이벤트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotUIUpdated, int32);

// 드래그 세션 종료 이벤트 (드래그를 시작한 위젯이 시각 복제본을 정리할 수 있도록 알림)
DECLARE_MULTICAST_DELEGATE(FOnDragEnded);

/**
 * 인벤토리 ViewModel (UDC1ViewModelSubsystem에서 생성/관리)
 * 플레이어 / 귀중품 인벤토리는 BindSource(PlayerState)에서 연결, 상자는 상자 화면이 열릴 때 SetTargetChest로 연결
 * LocalPlayer당 인스턴스 1개 - ESC 인벤토리와 상자 화면이 공유하여 드래그 세션이 상호 연동됨
 */
UCLASS()
class DC1_API UDC1InventoryViewModel : public UDC1ViewModelBase
{
	GENERATED_BODY()

public:
	virtual void BindSource(ADC1PlayerState* InPlayerState) override;
	virtual void ClearSource() override;

public:
	/** 전체 인벤토리 갱신 이벤트 (레거시, 사용 권장 안함) */
	FOnInventoryUpdated OnInventoryUpdated;

	/** 특정 슬롯만 갱신하는 이벤트 (성능 최적화) */
	FOnSlotUIUpdated OnSlotUIUpdated;

	/** 상자 전용: 전체 갱신 이벤트 */
	FOnInventoryUpdated OnChestInventoryUpdated;

	/** 상자 전용: 특정 슬롯만 갱신하는 이벤트 */
	FOnSlotUIUpdated OnChestSlotUIUpdated;

	/** 귀중품 전용: 전체 갱신 이벤트 (슬롯 수가 바뀔 수 있으므로 위젯은 칸 수를 다시 맞춰야 함) */
	FOnInventoryUpdated OnValuableInventoryUpdated;

	/** 귀중품 전용: 특정 슬롯만 갱신하는 이벤트 */
	FOnSlotUIUpdated OnValuableSlotUIUpdated;

	/** 드래그 세션 종료 이벤트 */
	FOnDragEnded OnDragEnded;

	// 장비 칸 갱신 이벤트
	FOnInventoryUpdated OnEquipmentUIUpdated;

public:
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

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetValuableSlotCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	const FDC1InventorySlotInfo& GetValuableSlot(int32 Index) const;

	/** 출처(플레이어/귀중품/상자)에 맞는 슬롯 조회 - 슬롯 위젯 공용 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	const FDC1InventorySlotInfo& GetSlotBySource(EDC1InventorySource InSource, int32 Index) const;

	/** 출처(플레이어/귀중품/상자)에 맞는 슬롯 개수 조회 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetSlotCountBySource(EDC1InventorySource InSource) const;

	const FDC1ItemRow* GetItemData(FName ItemID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Equipment")
	int32 GetEquipmentSlotCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Equipment")
	const FDC1InventorySlotInfo& GetEquipmentSlot(int32 Index) const;

	// 장비 칸 타입 조회 함수 (빈 칸 아이콘 표시용)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Equipment")
	EDC1EquipSlotType GetEquipSlotType(int32 Index) const;

	// 아이템이 장비 칸에 맞는지 확인하는 함수 (드롭 가능 표시용)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Equipment")
	bool CanEquipToSlot(FName ItemID, int32 EquipSlotIndex) const;

	// 장비 칸 잠김 여부 조회 함수 (사선 잠금 표시용)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Equipment")
	bool IsEquipSlotLocked(int32 EquipSlotIndex) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Weight")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Weight")
	float GetMaxWeight() const;

	// Item.Money 카테고리 아이템 개수 합계 (칸마다 int 최대까지 쌓이므로 int64)
	int64 GetMoneyCount() const;

	// 드래그 중인 아이템을 버리는 함수 (플레이어 인벤토리에서 시작한 드래그만)
	UFUNCTION(BlueprintCallable, Category = "Inventory|Drag")
	void RequestDropDraggedItem();

	// 드래그 중인 아이템을 폐기하는 함수 (플레이어 인벤토리에서 시작한 드래그만)
	UFUNCTION(BlueprintCallable, Category = "Inventory|Drag")
	void RequestDiscardDraggedItem();

	// 장비 장착/해제 요청 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void RequestEquipItem(int32 SlotIndex, int32 EquipSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void RequestEquipItemAuto(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void RequestUnequipItem(int32 EquipSlotIndex, int32 ToSlotIndex = -1);

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
	void RequestTransferItem(bool bToChest, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount = -1);

	/** 플레이어 인벤토리 일회성 정렬 요청 (분류 버튼) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestSortInventory();

	/** 현재 열린 상자 일회성 정렬 요청 (분류 버튼) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestSortChest();

	/** 귀중품 일회성 정렬 요청 (귀중품 탭 분류 버튼) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestSortValuables();

public:
	// ========================
	// 드래그 세션 (플레이어 위젯 / 상자 위젯이 동일 인스턴스를 공유하여 서로의 드래그를 인지)
	// ========================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Drag")
	bool IsDragging() const { return bIsDragging; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Drag")
	bool IsDragSourceChest() const { return bDragSourceIsChest; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Drag")
	bool IsDragSourceEquipment() const { return bDragSourceIsEquipment; }

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

	// 장비 칸에서 드래그를 시작하는 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory|Drag")
	void BeginDragFromEquipment(int32 EquipSlotIndex);

	// 드래그 중인 아이템을 장비 칸에 드롭하는 함수 (인벤토리에서 시작한 드래그만 장착)
	UFUNCTION(BlueprintCallable, Category = "Inventory|Drag")
	void RequestDropToEquipment(int32 EquipSlotIndex);

protected:
	/** 플레이어 인벤토리 구독 교체 (BindSource/ClearSource 전용) */
	void SetPlayerInventoryComponent(UDC1PlayerInventoryComponent* InPlayerComp);

	/** 귀중품 인벤토리 구독 교체 (BindSource/ClearSource 전용) */
	void SetValuableInventoryComponent(UDC1ValuableInventoryComponent* InValuableComp);

	void HandleInventoryUpdated();
	void HandleSlotUpdated(int32 SlotIndex);
	void HandleChestInventoryUpdated();
	void HandleChestSlotUpdated(int32 SlotIndex);
	void HandleValuableInventoryUpdated();
	void HandleValuableSlotUpdated(int32 SlotIndex);
	void HandleEquipmentUpdated();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UDC1PlayerInventoryComponent> MyInventoryComp;

	/** 현재 열린 상자 컴포넌트 (nullptr이면 없음) */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UDC1ChestInventoryComponent> TargetChestComp;

	/** 귀중품 인벤토리 (드래그 불가, 정렬만 가능) */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UDC1ValuableInventoryComponent> ValuableInventoryComp;

private:
	bool bIsDragging = false;
	bool bDragSourceIsChest = false;
	// 장비 칸에서 시작한 드래그인지 (true면 DragSourceSlotIndex는 장비 칸 인덱스)
	bool bDragSourceIsEquipment = false;
	int32 DragSourceSlotIndex = -1;
	int32 DragStackCount = 0;
	const FDC1ItemRow* DragItemData = nullptr;
};
