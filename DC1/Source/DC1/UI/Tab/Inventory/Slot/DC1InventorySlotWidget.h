// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Component/Inventory/DC1InventoryTypes.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1Enums.h"
#include "DC1InventorySlotWidget.generated.h"

class UBorder;
class USizeBox;
class UImage;
class UTextBlock;
class UWidgetAnimation;
class UTexture2D;
class USoundBase;

class UDC1InventoryEntryWidget;
class UDC1InventoryTooltipWidget;

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
	void InitSlot(int32 SlotIndex, float SlotSize, EDC1InventorySource InSlotSource = EDC1InventorySource::Player);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetSlotData(const UDC1InventoryViewModel* ViewModel);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearSlot();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	UDC1InventoryEntryWidget* GetEntryWidget() const { return EntryWidget; }

	/**
	 * 무게 텍스트 표시 제어
	 * @param bVisible 표시 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetWeightVisibility(bool bVisible);

	/**
	 * 드래그용 복제본 위젯 초기화
	 * C++에서 생성된 복제본이 제대로 렌더링되도록 설정
	 */
	void InitializeDragCopy();

	/**
	 * 표시되는 개수를 덮어씀 (Ctrl/Shift로 부분 선택된 개수를 드래그 복제본에 표시할 때 사용)
	 * @param NewCount 표시할 개수
	 */
	void OverrideDisplayCount(int32 NewCount);

	// 빈 칸일 때 보여줄 아이콘을 지정하는 함수 (장비 칸 실루엣용)
	void SetEmptyIcon(UTexture2D* InIcon);

	// 잠긴 칸 표시(사선)를 켜고 끄는 함수
	void SetLocked(bool bInLocked);

protected:
	void UpdateRarityColor(EDC1ItemRarity Rarity);
	void ShowHoverImage();
	void HideHoverImage();

	// 빈 칸 아이콘 표시 여부를 갱신하는 함수
	void UpdateEmptyIconVisibility(bool bIsEmpty);

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

	// 빈 칸 실루엣 아이콘 (장비 칸용, WBP에 없어도 됨)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_EmptyIcon;

	// 잠긴 칸 사선 이미지 (장비 칸용, WBP에 없어도 됨)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Lock;

	// 등급 그라데이션 (있으면 바탕은 고정색, 등급은 이 이미지 틴트로 표시)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Rarity;

private:
	int32 CurrentSlotIndex = -1;
	bool bIsBeingDragged = false;
	EDC1InventorySource SlotSource = EDC1InventorySource::Player;

	UPROPERTY(EditAnywhere, Category = "Inventory|Tooltip")
	TSubclassOf<UDC1InventoryTooltipWidget> TooltipClass;

	UPROPERTY(Transient)
	TObjectPtr<UDC1InventoryTooltipWidget> CachedTooltip = { nullptr };

	// 슬롯은 UButton이 아니라 Style 사운드가 없음 -> MouseEnter에서 직접 재생
	UPROPERTY(EditAnywhere, Category = "Inventory|Sound")
	TObjectPtr<USoundBase> HoverSound = { nullptr };
};
