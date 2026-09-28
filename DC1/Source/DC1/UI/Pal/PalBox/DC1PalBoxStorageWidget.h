// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "ViewModel/Pal/DC1PalBoxDisplayTypes.h"
#include "UI/Pal/PalBox/DC1PalBoxSlotWidget.h"
#include "DC1PalBoxStorageWidget.generated.h"

class UUniformGridPanel;
class UDC1PalBoxStorageSlotWidget;

/**
 * 상자 패널. 6열 그리드
 */
UCLASS()
class DC1_API UDC1PalBoxStorageWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void			CreateSlots(int32 InCapacity);
	void			SetSlotInfo(int32 InSlotIndex, const FDC1PalBoxSlotInfo& InSlotInfo);
	void			SetEmpty(int32 InSlotIndex);

	const TArray<TObjectPtr<UDC1PalBoxStorageSlotWidget>>&	GetSlotWidgets() const { return SlotWidgets; }

public:
	FOnPalBoxSlotHovered	OnSlotHovered;

private:
	void			HandleSlotHovered(int32 InSlotIndex);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UUniformGridPanel>	SlotGrid = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "PalBox")
	TSubclassOf<UDC1PalBoxStorageSlotWidget>	SlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "PalBox", meta = (ClampMin = "1"))
	int32							Columns = { 6 };

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UDC1PalBoxStorageSlotWidget>>	SlotWidgets;
};
