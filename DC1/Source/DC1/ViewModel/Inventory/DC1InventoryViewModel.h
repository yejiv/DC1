// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Component/Inventory/DC1InventoryTypes.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "DC1InventoryViewModel.generated.h"

UCLASS()
class DC1_API UDC1InventoryViewModel : public UObject
{
	GENERATED_BODY()

public:
	UDC1InventoryViewModel();

public:
	// 슬롯 관련
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	const TArray<FDC1InventorySlot>& GetInventorySlots() const { return InventorySlots; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetSlotCount() const { return InventorySlots.Num(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	const FDC1InventorySlot& GetSlot(int32 Index) const;

	// 아이템 정보 조회 (C++ 전용)
	const FDC1ItemRow* GetItemData(const FName& ItemID) const;

protected:
	// Mock 데이터 초기화
	void InitializeMockData();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FDC1InventorySlot> InventorySlots;

	// Mock 데이터 저장소
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TMap<FName, FDC1ItemRow> ItemDatabase;
};
