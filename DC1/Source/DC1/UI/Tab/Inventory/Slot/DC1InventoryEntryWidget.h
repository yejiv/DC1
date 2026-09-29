// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Define/DC1Enums.h"
#include "DC1InventoryEntryWidget.generated.h"

class USizeBox;
class UTextBlock;
class UImage;
class UDC1InventorySlotWidget;
class UDC1InventoryViewModel;

UCLASS()
class DC1_API UDC1InventoryEntryWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	UDC1InventoryEntryWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetEntryData(const UDC1InventoryViewModel* ViewModel, int32 SlotIndex, EDC1InventorySource InSlotSource = EDC1InventorySource::Player);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearEntry();

	// 멤버 변수 접근 함수
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FName GetCurrentItemID() const { return CurrentItemID; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetCurrentStackCount() const { return CurrentStackCount; }

	/**
	 * 무게 텍스트 표시 제어
	 * @param bVisible 표시 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetWeightVisibility(bool bVisible);

	/**
	 * 표시되는 개수만 별도로 덮어씀 (실제 슬롯 데이터는 변경하지 않음)
	 * 드래그 복제본에서 부분 선택된 개수(Ctrl/Shift)를 표시할 때 사용
	 * @param NewItemCount 표시할 개수
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshItemCount(int32 NewItemCount);

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Count;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Weight;

private:
	// 아이템 정보 캐시 (UI 표시용)
	int32 CurrentSlotIndex = -1;
	FName CurrentItemID = NAME_None;
	int32 CurrentStackCount = 0;
	float CurrentItemWeight = 0.0f;
};
