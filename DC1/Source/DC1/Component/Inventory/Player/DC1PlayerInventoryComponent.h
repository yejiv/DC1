// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/Inventory/DC1InventoryComponentBase.h"
#include "DC1PlayerInventoryComponent.generated.h"

class UDC1ChestInventoryComponent;
class UDC1ValuableInventoryComponent;

// 플레이어 인벤토리 (무게/장비 칸 포함, PlayerState 부착, COND_OwnerOnly 복제)

UCLASS()
class DC1_API UDC1PlayerInventoryComponent : public UDC1InventoryComponentBase
{
	GENERATED_BODY()

public:
	UDC1PlayerInventoryComponent();

public:
	// 장비 칸 갱신 이벤트 (UI용)
	FOnInventoryUpdated OnEquipmentUpdated;

	// 특정 장비 칸 변경 이벤트 (서버 전용, EquipComponent가 구독)
	FOnSlotChanged OnEquipmentSlotChanged;

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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

	// 아이템이 장비 칸 타입에 맞고 열린 칸인지 확인하는 함수
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Equipment")
	bool CanEquipToSlot(FName InItemID, int32 InEquipSlotIndex) const;

	// 타입별 열린 칸 수 조회 함수 (머리/갑옷/방패/글라이더는 항상 전부 열림)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Equipment")
	int32 GetUnlockedSlotCount(EDC1EquipSlotType InSlotType) const;

	// 장비 칸이 열려 있는지 확인하는 함수
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Equipment")
	bool IsEquipSlotUnlocked(int32 InEquipSlotIndex) const;

	// 무기/장신구 열린 칸 수를 바꾸는 함수 (서버, 해금 조건은 추후 연결)
	void SetUnlockedSlotCount(EDC1EquipSlotType InSlotType, int32 InCount);

public:
	// 아이템 추가 함수 (귀중품은 귀중품 인벤토리로 넘김)
	virtual bool AddItem(FName InItemID, int32 InCount = 1) override;

	// 보관 가능 여부 확인 함수 (귀중품 불가)
	virtual bool CanAcceptItem(FName InItemID) const override;

public:
	// 지정한 장비 칸에 장착하는 함수 (서버, 차 있으면 교환)
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool EquipItem(int32 InSlotIndex, int32 InEquipSlotIndex);

	// 맞는 장비 칸에 자동 장착하는 함수 (서버, 우클릭용)
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool EquipItemAuto(int32 InSlotIndex);

	// 장비 해제 함수 (서버, InToSlotIndex -1이면 첫 빈 칸, 차 있으면 맞는 장비일 때만 교환)
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	bool UnequipItem(int32 InEquipSlotIndex, int32 InToSlotIndex = -1);

	// 장비 장착/해제 요청 함수 (클라 -> 서버 RPC)
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void RequestEquipItem(int32 InSlotIndex, int32 InEquipSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void RequestEquipItemAuto(int32 InSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
	void RequestUnequipItem(int32 InEquipSlotIndex, int32 InToSlotIndex = -1);

	// 아이템 버리기 함수 (서버, 지금은 폐기와 동일)
	bool DropItem(int32 InSlotIndex, int32 InCount);

	// 아이템 폐기 함수 (서버)
	bool DiscardItem(int32 InSlotIndex, int32 InCount);

	// 버리기/폐기 요청 함수 (클라 -> 서버 RPC)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestDropItem(int32 InSlotIndex, int32 InCount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestDiscardItem(int32 InSlotIndex, int32 InCount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestMoveItem(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount = -1);

	// 상자 내부 슬롯 이동 요청 함수 (상자 UI 드래그 앤 드롭)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestMoveChestItem(UDC1ChestInventoryComponent* InChestComp, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount = -1);

	// 플레이어 <-> 상자 아이템 이전 요청 함수 (bToChest 방향 기준으로 From/To 인덱스 해석)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestTransferItem(UDC1ChestInventoryComponent* InChestComp, bool bToChest, int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount = -1);

	// 인벤토리 정렬 요청 함수 (분류 버튼)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestSortInventory();

	// 귀중품 정렬 요청 함수 (분류 버튼)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestSortValuables();

	// 상자 정렬 요청 함수 (분류 버튼)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RequestSortChestItems(UDC1ChestInventoryComponent* InChestComp);

protected:
	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_MoveItem(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount);

	bool Server_MoveItem_Validate(int32 FromSlotIndex, int32 ToSlotIndex, int32 MoveCount);

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

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	void Server_EquipItem(int32 InSlotIndex, int32 InEquipSlotIndex);

	bool Server_EquipItem_Validate(int32 InSlotIndex, int32 InEquipSlotIndex);

	void Server_EquipItem_Implementation(int32 InSlotIndex, int32 InEquipSlotIndex);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	void Server_EquipItemAuto(int32 InSlotIndex);

	bool Server_EquipItemAuto_Validate(int32 InSlotIndex);

	void Server_EquipItemAuto_Implementation(int32 InSlotIndex);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory|Equipment")
	void Server_UnequipItem(int32 InEquipSlotIndex, int32 InToSlotIndex);

	bool Server_UnequipItem_Validate(int32 InEquipSlotIndex, int32 InToSlotIndex);

	void Server_UnequipItem_Implementation(int32 InEquipSlotIndex, int32 InToSlotIndex);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_DropItem(int32 InSlotIndex, int32 InCount);

	bool Server_DropItem_Validate(int32 InSlotIndex, int32 InCount);

	void Server_DropItem_Implementation(int32 InSlotIndex, int32 InCount);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Inventory")
	void Server_DiscardItem(int32 InSlotIndex, int32 InCount);

	bool Server_DiscardItem_Validate(int32 InSlotIndex, int32 InCount);

	void Server_DiscardItem_Implementation(int32 InSlotIndex, int32 InCount);

protected:
	virtual void CalculateWeight();
	virtual void InitializeEquipmentSlots();
	virtual void InitializeDefaultItems();

	// 시작 무기를 무기 칸 앞에서부터 장착하는 함수 (서버)
	void InitializeDefaultEquipment();
	virtual void InitializeSlots() override;
	virtual void OnInventoryUpdated_Internal() override;

	// 같은 PlayerState의 귀중품 인벤토리 조회 함수
	UDC1ValuableInventoryComponent* FindValuableInventory() const;

	// 장비 칸 복제 수신 시 UI 갱신 이벤트를 보내는 함수
	UFUNCTION()
	void OnRep_EquipmentSlots();

	// 아이템이 들어갈 장비 칸을 찾는 함수 (빈 칸 우선, 없으면 첫 칸, 불가면 -1)
	int32 FindEquipSlotForItem(FName InItemID) const;

	// 장비 칸 변경 후 무게 갱신과 장비 이벤트를 보내는 함수 (서버)
	void HandleEquipmentChanged(int32 InEquipSlotIndex);

	// 가진 먹이 주머니 중 가장 큰 값으로 음식 칸 수를 다시 계산하는 함수 (서버)
	void RefreshFoodSlotCount();

	// 귀중품 인벤토리 변경 시 음식 칸 수를 다시 계산하는 함수 (서버)
	void HandleValuableInventoryUpdated();

	// 열린 칸 수 복제 수신 시 장비 UI 갱신 이벤트를 보내는 함수
	UFUNCTION()
	void OnRep_UnlockedSlotCounts();

	// 무게 복제 수신 시 인벤토리 UI 갱신 이벤트를 보내는 함수 (무게 게이지용)
	UFUNCTION()
	void OnRep_CurrentWeight();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Weight")
	float MaxWeight = 100.0f;

	// 시작할 때 무기 칸에 장착할 아이템 ID (순서대로 무기 1, 2 ...)
	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Equipment")
	TArray<FName> DefaultWeaponItemIDs;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeight, BlueprintReadOnly, Category = "Inventory|Weight")
	float CurrentWeight = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_EquipmentSlots, BlueprintReadOnly, Category = "Inventory|Equipment")
	TArray<FDC1InventorySlotInfo> EquipmentSlots;

	// 타입별 열린 칸 수 (앞에서부터 이 수만큼 열림)
	UPROPERTY(ReplicatedUsing = OnRep_UnlockedSlotCounts)
	int32 UnlockedWeaponCount = { DC1EquipSlotLayout::DefaultUnlockedWeaponCount };

	UPROPERTY(ReplicatedUsing = OnRep_UnlockedSlotCounts)
	int32 UnlockedAccessoryCount = { DC1EquipSlotLayout::DefaultUnlockedAccessoryCount };

	// 먹이 주머니로 열리는 음식 칸 수
	UPROPERTY(ReplicatedUsing = OnRep_UnlockedSlotCounts)
	int32 UnlockedFoodCount = { 0 };
};
