// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/Inventory/DC1InventoryComponentBase.h"
#include "DC1PlayerInventoryComponent.generated.h"

/**
 * 플레이어 인벤토리 컴포넌트
 * 기본 인벤토리에 무게, 장비 슬롯 등의 플레이어 전용 기능 추가
 *
 * APlayerState에 부착되어 플레이어별 인벤토리 관리
 * 복제 설정: COND_OwnerOnly (플레이어 본인과 서버만 데이터 공유)
 */
UCLASS()
class DC1_API UDC1PlayerInventoryComponent : public UDC1InventoryComponentBase
{
	GENERATED_BODY()

public:
	UDC1PlayerInventoryComponent();

public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Weight")
	float GetCurrentWeight() const { return CurrentWeight; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Weight")
	float GetMaxWeight() const { return MaxWeight; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Weight")
	float GetWeightPercentage() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Weight")
	float GetRemainingWeight() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Weight")
	bool CanAddItem(FName InItemID, int32 InCount = 1) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Equipment")
	const TArray<FDC1InventorySlot>& GetEquipmentSlots() const { return EquipmentSlots; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Equipment")
	const FDC1InventorySlot& GetEquipmentSlot(int32 InSlotIndex) const;

	// 장비 착용 및 해제 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool EquipItem(int32 InSlotIndex, int32 InEquipSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool UnequipItem(int32 InEquipSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SortInventory();

protected:
	virtual void CalculateWeight();
	virtual void InitializeEquipmentSlots();
	virtual void InitializeSlots() override;
	virtual void OnInventoryUpdated_Internal() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weight")
	float MaxWeight = 100.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Weight")
	float CurrentWeight = 0.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Equipment")
	TArray<FDC1InventorySlot> EquipmentSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
	bool bAutoSort = false;

private:
	/** 개별 아이템의 무게 (임시, 데이터 연동 필요) */
	UPROPERTY(EditAnywhere, Category = "Inventory|Weight")
	float ItemWeightPerUnit = 1.0f;
};
