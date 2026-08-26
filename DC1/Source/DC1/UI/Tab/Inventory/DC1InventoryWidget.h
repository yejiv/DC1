// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1InventoryWidget.generated.h"

class UDC1InventoryViewModel;
class UDC1InventorySlotsWidget;
class UTextBlock;
class UBorder;
class UVerticalBox;

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
};
