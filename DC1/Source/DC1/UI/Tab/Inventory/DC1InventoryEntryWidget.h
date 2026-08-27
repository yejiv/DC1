// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
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
	void SetEntryData(const UDC1InventoryViewModel* ViewModel, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearEntry();

	// 멤버 변수 접근 함수
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FName GetCurrentItemID() const { return CurrentItemID; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetCurrentStackCount() const { return CurrentStackCount; }

protected:
	virtual void NativeConstruct() override;

private:
	void RefreshItemCount(int32 NewItemCount);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Count;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

private:
	// 아이템 정보 캐시 (UI 표시용)
	int32 CurrentSlotIndex = -1;
	FName CurrentItemID = NAME_None;
	int32 CurrentStackCount = 0;
};
