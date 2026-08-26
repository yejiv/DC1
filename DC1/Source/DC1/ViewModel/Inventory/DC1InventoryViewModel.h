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

UCLASS()
class DC1_API UDC1InventoryViewModel : public UObject
{
	GENERATED_BODY()

public:
	UDC1InventoryViewModel();

public:
	FOnInventoryUpdated OnInventoryUpdated;

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
	const TArray<FDC1InventorySlot>& GetInventorySlots() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetSlotCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	const FDC1InventorySlot& GetSlot(int32 Index) const;

	const FDC1ItemRow* GetItemData(FName ItemID) const;

protected:
	void HandleInventoryUpdated();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UDC1PlayerInventoryComponent> MyInventoryComp;

	/** 현재 열린 상자 컴포넌트 (nullptr이면 없음) */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UDC1ChestInventoryComponent> TargetChestComp;
};
