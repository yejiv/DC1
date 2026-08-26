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
	void InitSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetSlotData(const UDC1InventoryViewModel* ViewModel);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearSlot();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	UDC1InventoryEntryWidget* GetEntryWidget() const { return EntryWidget; }

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
};
