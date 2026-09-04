// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Component/Inventory/DC1InventoryTypes.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "DC1InventorySlotWidget.generated.h"

class UBorder;
class USizeBox;
class UImage;
class UTextBlock;
class UWidgetAnimation;

class UDC1InventoryEntryWidget;

UCLASS()
class DC1_API UDC1InventorySlotWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

public:
	void InitSlot(int32 SlotIndex, float SlotSize, bool bInIsChestSlot = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetSlotData(const UDC1InventoryViewModel* ViewModel);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearSlot();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	UDC1InventoryEntryWidget* GetEntryWidget() const { return EntryWidget; }

	/**
	 * 무게 텍스트 표시 제어
	 * @param bVisible 표시 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetWeightVisibility(bool bVisible);

	/**
	 * 드래그용 복제본 위젯 초기화
	 * C++에서 생성된 복제본이 제대로 렌더링되도록 설정
	 */
	void InitializeDragCopy();

	/**
	 * 표시되는 개수를 덮어씀 (Ctrl/Shift로 부분 선택된 개수를 드래그 복제본에 표시할 때 사용)
	 * @param NewCount 표시할 개수
	 */
	void OverrideDisplayCount(int32 NewCount);

protected:
	void UpdateRarityColor(EDC1ItemRarity Rarity);
	void ShowHoverImage();
	void HideHoverImage();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> SlotBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Bracket;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation>	HoverAnim = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventoryEntryWidget> EntryWidget;

private:
	int32 CurrentSlotIndex = -1;
	bool bIsBeingDragged = false;
	bool bIsChestSlot = false;
};
