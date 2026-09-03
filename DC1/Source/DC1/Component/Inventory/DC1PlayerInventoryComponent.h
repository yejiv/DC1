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

public:
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
	const TArray<FDC1InventorySlotInfo>& GetEquipmentSlots() const { return EquipmentSlots; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Equipment")
	const FDC1InventorySlotInfo& GetEquipmentSlot(int32 InSlotIndex) const;

public:
	// 장비 착용 및 해제 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool EquipItem(int32 InSlotIndex, int32 InEquipSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool UnequipItem(int32 InEquipSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SortInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestMoveItem(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount = -1);

protected:
	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_MoveItem(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount);

	/** 서버 RPC 검증 함수 */
	bool Server_MoveItem_Validate(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount);

	/** 서버 RPC 구현 함수 */
	void Server_MoveItem_Implementation(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount);

protected:
	virtual void CalculateWeight();
	virtual void InitializeEquipmentSlots();
	virtual void InitializeDefaultItems();
	virtual void InitializeSlots() override;
	virtual void OnInventoryUpdated_Internal() override;

	/** 클라이언트에서 EquipmentSlots 복제 수신 시 호출되어 UI 갱신 이벤트를 발생시킴 */
	UFUNCTION()
	void OnRep_EquipmentSlots();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weight")
	float MaxWeight = 100.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Weight")
	float CurrentWeight = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_EquipmentSlots, BlueprintReadOnly, Category = "Inventory|Equipment")
	TArray<FDC1InventorySlotInfo> EquipmentSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Settings")
	bool bAutoSort = false;

private:
	UPROPERTY(EditAnywhere, Category = "Inventory|Weight")
	float ItemWeightPerUnit = 1.0f;
};
