// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Define/DC1Enums.h"
#include "DC1EquipmentWidget.generated.h"

class UDC1InventoryViewModel;
class UDC1InventorySlotWidget;
class UTexture2D;

// 팰월드 스타일 장비창 (장비 칸 19개 표시, 입력은 부모 InventoryWidget이 처리)
UCLASS()
class DC1_API UDC1EquipmentWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	// 마우스 아래의 장비 칸 인덱스를 구하는 함수 (없으면 -1)
	int32 GetEquipSlotIndexFromPosition(const FVector2D& InScreenPos) const;

	void SetViewModel(UDC1InventoryViewModel* InViewModel);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	// 장비 칸 전체를 VM 값으로 갱신하는 함수
	void RefreshAllSlots();

	void HandleEquipmentUpdated();

private:
	// 무기
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Weapon1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Weapon2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Weapon3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Weapon4;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Weapon5;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Weapon6;

	// 방어구
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Head;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Body;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Shield;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Glider;

	// 장신구
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Accessory1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Accessory2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Accessory3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Accessory4;

	// 음식
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Food1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Food2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Food3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Food4;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotWidget> InventorySlot_Food5;

	// 빈 칸 실루엣 아이콘 (칸 타입별)
	UPROPERTY(EditAnywhere, Category = "Equipment")
	TMap<EDC1EquipSlotType, TObjectPtr<UTexture2D>> EmptySlotIcons;

	UPROPERTY(EditAnywhere, Category = "Equipment")
	float SlotSize = { 80.f };

	// 장비 칸 인덱스 순서(DC1EquipSlotLayout)로 정렬한 슬롯 목록
	UPROPERTY()
	TArray<TObjectPtr<UDC1InventorySlotWidget>> EquipSlots;

	UPROPERTY()
	TObjectPtr<UDC1InventoryViewModel> CachedViewModel = { nullptr };
};
