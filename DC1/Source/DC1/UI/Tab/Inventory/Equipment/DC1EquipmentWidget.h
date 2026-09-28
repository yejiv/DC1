// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1EquipmentWidget.generated.h"

class UDC1InventoryViewModel;
class UBorder;
class UTextBlock;
class UProgressBar;

UCLASS()
class DC1_API UDC1EquipmentWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetViewModel(UDC1InventoryViewModel* NewViewModel);

protected:
	void CreateUI();

private:
	// 장비 슬롯
	UPROPERTY()
	TArray<TObjectPtr<UBorder>> WeaponSlots;

	UPROPERTY()
	TObjectPtr<UBorder> HeadSlot;

	UPROPERTY()
	TObjectPtr<UBorder> ChestSlot;

	UPROPERTY()
	TObjectPtr<UBorder> ShieldSlot;

	UPROPERTY()
	TObjectPtr<UBorder> GliderSlot;

	UPROPERTY()
	TArray<TObjectPtr<UBorder>> AccessorySlots;

	// 플레이어 정보
	UPROPERTY()
	TObjectPtr<UTextBlock> PlayerNameText;

	UPROPERTY()
	TObjectPtr<UTextBlock> PlayerLevelText;

	UPROPERTY()
	TObjectPtr<UDC1InventoryViewModel> CachedViewModel;
};
