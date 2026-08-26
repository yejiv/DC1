// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "DC1DragDropOperation.generated.h"

/**
 * 인벤토리 아이템 드래그 드롭 오퍼레이션
 * 출발 슬롯의 아이템 정보를 저장하여 드롭 시 전달
 */
UCLASS()
class DC1_API UDC1DragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UDC1DragDropOperation();

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory|DragDrop")
	void SetSourceSlot(int32 InSourceSlotIndex) { SourceSlotIndex = InSourceSlotIndex; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|DragDrop")
	int32 GetSourceSlot() const { return SourceSlotIndex; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|DragDrop")
	void SetSourceItemID(FName InItemID) { SourceItemID = InItemID; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|DragDrop")
	FName GetSourceItemID() const { return SourceItemID; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|DragDrop")
	void SetStackCount(int32 InStackCount) { StackCount = InStackCount; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|DragDrop")
	int32 GetStackCount() const { return StackCount; }

private:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|DragDrop", meta = (AllowPrivateAccess = "true"))
	int32 SourceSlotIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|DragDrop", meta = (AllowPrivateAccess = "true"))
	FName SourceItemID = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|DragDrop", meta = (AllowPrivateAccess = "true"))
	int32 StackCount = 0;
};
