// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1InventoryWidget.generated.h"

class UDC1InventoryViewModel;
class UDC1InventorySlotsWidget;
class UDC1ItemDragWidget;

class UTextBlock;
class UBorder;
class UVerticalBox;

struct FDC1ItemRow;

UCLASS()
class DC1_API UDC1InventoryWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	UDC1InventoryWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetViewModel(UDC1InventoryViewModel* NewViewModel);

protected:
	void HandleInventoryUpdated();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> InventoryBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> MainPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> InventoryTitle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotsWidget> InventorySlotsWidget;

	UPROPERTY()
	TObjectPtr<UDC1InventoryViewModel> CachedViewModel;

private:
	// 선택된 아이템 정보
	int32 DragSourceSlotIndex = -1;
	int32 DragStackCount = 0;
	const FDC1ItemRow* DragItemData = nullptr;

	// 드래그 위젯
	UPROPERTY()
	TObjectPtr<UDC1ItemDragWidget> DragWidgetContainer = nullptr;
};
