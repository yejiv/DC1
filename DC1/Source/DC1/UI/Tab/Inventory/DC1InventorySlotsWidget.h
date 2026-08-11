// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1InventorySlotsWidget.generated.h"


class UDC1InventoryViewModel;
class UDC1InventorySlotWidget;
class UScrollBox;
class UWrapBox;
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
	// ViewModel ¼³Á¤
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetViewModel(UDC1InventoryViewModel* NewViewModel);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshSlotCapacity(int32 NewTotalSlots);

protected:
	void CreateSlotWidgets(int32 TotalSlots);

	UFUNCTION()
	void HandleInventoryUpdated();

private:
	static constexpr int32 InventoryColumns = 5;
	static constexpr int32 DefaultSlotCapacity = 35;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> InventoryScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> SlotGrid;

	UPROPERTY()
	TObjectPtr<UDC1InventoryViewModel> CachedViewModel;

	UPROPERTY()
	TArray<TObjectPtr<UDC1InventorySlotWidget>> SlotWidgets;

	UPROPERTY(EditAnywhere, Category = "Inventory|Settings")
	TSubclassOf<UDC1InventorySlotWidget> SlotWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Inventory|Settings")
	FMargin SlotMargin = FMargin(2.0f);
};
