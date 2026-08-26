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
class UDC1InventoryViewModel;

UCLASS()
class DC1_API UDC1InventorySlotWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetSlotData(const UDC1InventoryViewModel* ViewModel, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearSlot();

protected:
	void UpdateSlotUI();
	void UpdateRarityColor(EDC1ItemRarity Rarity);
	void CreateUI();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> SlotBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StackCountText;

	int32 CurrentSlotIndex = -1;
	FDC1InventorySlot CurrentSlot;
};
