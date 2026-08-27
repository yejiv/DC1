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

protected:
	void HandleInventoryUpdated();
	void HandleSlotUpdated(int32 SlotIndex);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UDC1PlayerInventoryComponent> MyInventoryComp;

	/** 현재 열린 상자 컴포넌트 (nullptr이면 없음) */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UDC1ChestInventoryComponent> TargetChestComp;
};
