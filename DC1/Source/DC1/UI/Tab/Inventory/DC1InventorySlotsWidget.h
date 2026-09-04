// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1InventorySlotsWidget.generated.h"


class UDC1InventoryViewModel;
class UDC1InventorySlotWidget;
class UDC1InventoryEntryWidget;
class UScrollBox;
class UUniformGridPanel;

UCLASS()
class DC1_API UDC1InventorySlotsWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	UDC1InventorySlotsWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetSlotIndexFromPosition(FVector2D MousePosition) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetInventoryColumns() const { return InventoryColumns; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetDefaultSlotCapacity() const { return DefaultSlotCapacity; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	float GetSlotSize() const { return SlotSize; }

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetInventoryViewModel(UDC1InventoryViewModel* InViewModel);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshSlotCapacity(int32 NewTotalSlots);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshAllSlots();

	int32 CalculateSlotIndex(FVector2D ScreenSpaceMousePos);

protected:
	void CreateSlotWidgets(int32 TotalSlots);
	void UpdateSlotDisplay(int32 SlotIndex);

	void HandleInventoryUpdated();
	void HandleSlotUpdated(int32 SlotIndex);

private:
	static constexpr int32 InventoryColumns = 6;
	static constexpr int32 DefaultSlotCapacity = 45;
	static constexpr float SlotSize = 100.0f;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> InventoryScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> SlotGrid;

	UPROPERTY(EditAnywhere, Category = "Inventory|Settings")
	FMargin SlotMargin = FMargin(2.0f);

	/** true면 ViewModel의 상자 전용 데이터(GetChestSlot 등)를 조회, false면 플레이어 인벤토리 데이터를 조회 */
	UPROPERTY(EditAnywhere, Category = "Inventory|Settings")
	bool bIsChestSlots = false;


private:
	// 다른 위젯 트리에 인스턴스로 배치되므로(WBP 서브클래싱이 아님) 인스턴스별로 설정 가능해야 함 - EditAnywhere 필요
	UPROPERTY(EditAnywhere, Category = "Inventory|Settings")
	TSubclassOf<UDC1InventorySlotWidget> SlotWidgetClass;

	UPROPERTY()
	TArray<TObjectPtr<UDC1InventorySlotWidget>> SlotWidgets;

	UPROPERTY()
	TObjectPtr<UDC1InventoryViewModel> CachedViewModel;
};
