// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/Inventory/DC1InventoryComponentBase.h"
#include "DC1PlayerInventoryComponent.generated.h"

class UDC1ChestInventoryComponent;
class UDC1ValuableInventoryComponent;

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
	/** 귀중품(bIsValuable)은 같은 PlayerState의 귀중품 인벤토리로 넘기고, 일반 아이템만 여기에 보관 */
	virtual bool AddItem(FName InItemID, int32 InCount = 1) override;

	/** 귀중품은 일반 인벤토리에 보관 불가 */
	virtual bool CanAcceptItem(FName InItemID) const override;

public:
	// 장비 착용 및 해제 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool EquipItem(int32 InSlotIndex, int32 InEquipSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool UnequipItem(int32 InEquipSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestMoveItem(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount = -1);

	/**
	 * 상자 내부 슬롯 재배치 요청 (상자 UI에서의 드래그 앤 드롭용)
	 * @param InChestComp 대상 상자 컴포넌트
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestMoveChestItem(UDC1ChestInventoryComponent* InChestComp, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount = -1);

	/**
	 * 플레이어 인벤토리 <-> 상자 간 아이템 이전 요청
	 * @param InChestComp 대상 상자 컴포넌트
	 * @param bToChest true면 플레이어->상자, false면 상자->플레이어
	 * @param FromSlotIndex 출발 슬롯 (bToChest 기준으로 플레이어 또는 상자 쪽 인덱스)
	 * @param ToSlotIndex 드롭한 목표 슬롯 (bToChest 기준으로 상자 또는 플레이어 쪽 인덱스)
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestTransferItem(UDC1ChestInventoryComponent* InChestComp, bool bToChest, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount = -1);

	/** 플레이어 인벤토리 일회성 정렬 요청 (분류 버튼용) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestSortInventory();

	/** 귀중품 인벤토리 일회성 정렬 요청 (귀중품 탭 분류 버튼용) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestSortValuables();

	/**
	 * 상자 인벤토리 일회성 정렬 요청 (분류 버튼용)
	 * @param InChestComp 대상 상자 컴포넌트
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestSortChestItems(UDC1ChestInventoryComponent* InChestComp);

protected:
	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_MoveItem(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount);

	/** 서버 RPC 검증 함수 */
	bool Server_MoveItem_Validate(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount);

	/** 서버 RPC 구현 함수 */
	void Server_MoveItem_Implementation(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_MoveChestItem(UDC1ChestInventoryComponent* InChestComp, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount);

	bool Server_MoveChestItem_Validate(UDC1ChestInventoryComponent* InChestComp, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount);

	void Server_MoveChestItem_Implementation(UDC1ChestInventoryComponent* InChestComp, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_TransferItem(UDC1ChestInventoryComponent* InChestComp, bool bToChest, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount);

	bool Server_TransferItem_Validate(UDC1ChestInventoryComponent* InChestComp, bool bToChest, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount);

	void Server_TransferItem_Implementation(UDC1ChestInventoryComponent* InChestComp, bool bToChest, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_SortInventory();

	bool Server_SortInventory_Validate();

	void Server_SortInventory_Implementation();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_SortValuables();

	bool Server_SortValuables_Validate();

	void Server_SortValuables_Implementation();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_SortChestItems(UDC1ChestInventoryComponent* InChestComp);

	bool Server_SortChestItems_Validate(UDC1ChestInventoryComponent* InChestComp);

	void Server_SortChestItems_Implementation(UDC1ChestInventoryComponent* InChestComp);

protected:
	virtual void CalculateWeight();
	virtual void InitializeEquipmentSlots();
	virtual void InitializeDefaultItems();
	virtual void InitializeSlots() override;
	virtual void OnInventoryUpdated_Internal() override;

	/** 같은 PlayerState에 붙은 귀중품 인벤토리 */
	UDC1ValuableInventoryComponent* FindValuableInventory() const;

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

private:
	UPROPERTY(EditAnywhere, Category = "Inventory|Weight")
	float ItemWeightPerUnit = 1.0f;
};
