// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1InventoryEntryWidget.generated.h"

class USizeBox;
class UTextBlock;
class UImage;
class UDC1InventorySlotWidget;

UCLASS()
class DC1_API UDC1InventoryEntryWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	UDC1InventoryEntryWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetEntryData(const UDC1InventoryViewModel* ViewModel, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearEntry();

	void SetSlotWidget(UDC1InventorySlotWidget* InSlotWidget);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	void RefreshItemCount(int32 NewItemCount);

private:
	int32 CurrentSlotIndex = -1;

	UPROPERTY()
	TObjectPtr<UDC1InventorySlotWidget> SlotWidget;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Count;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;
};
