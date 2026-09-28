// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Define/DC1Enums.h"
#include "DC1InventoryWidget.generated.h"

class UDC1InventoryViewModel;
class UDC1InventorySlotsWidget;
class UDC1InventorySlotWidget;

class UBorder;
class UVerticalBox;
class UButton;

/**
 * I키 인벤토리 패널 (팰월드 형태)
 * 상단 탭 버튼(인벤토리 / 귀중품) + 우측 끝 분류 버튼, 하단 슬롯 그리드는 선택한 탭의 인벤토리를 표시
 * 귀중품 탭은 드래그 이동 불가 (상자 보관 / 버리기 불가), 분류 버튼으로만 정렬
 */
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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetViewModel(UDC1InventoryViewModel* NewViewModel);

protected:
	void HandleDragEnded();

	/** 탭 전환 - 슬롯 그리드 출처 교체, 진행 중 드래그 취소, 선택된 탭 버튼 표시 갱신 */
	void ChangeTab(EDC1InventorySource InTab);

	/** 선택된 탭 버튼은 밝게, 나머지는 어둡게 (배경색) */
	void UpdateTabButtonVisual();

	UFUNCTION()
	void HandleInventoryTabClicked();

	UFUNCTION()
	void HandleValuableTabClicked();

	/** 분류 버튼 - 현재 탭의 인벤토리 일회성 정렬 */
	UFUNCTION()
	void HandleSortButtonClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> InventoryBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> MainPanel;

	// 탭 버튼 (기존 제목 텍스트 자리)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_InventoryTab;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_ValuableTab;

	// 분류 버튼 (탭 버튼 줄 우측 끝)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Sort;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotsWidget> InventorySlotsWidget;

	UPROPERTY()
	TObjectPtr<UDC1InventoryViewModel> CachedViewModel;

	UPROPERTY(EditAnywhere, Category = "Inventory|Drag")
	TSubclassOf<UDC1InventorySlotWidget> DragSlotWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Inventory|Drag")
	float DragFollowSpeed = 15.0f;

	// 선택된 탭 버튼 배경색
	UPROPERTY(EditAnywhere, Category = "Inventory|Tab")
	FLinearColor SelectedTabColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// 선택되지 않은 탭 버튼 배경색
	UPROPERTY(EditAnywhere, Category = "Inventory|Tab")
	FLinearColor UnselectedTabColor = FLinearColor(0.25f, 0.25f, 0.25f, 1.0f);

	// 현재 선택된 탭 (Player = 인벤토리, Valuable = 귀중품)
	EDC1InventorySource CurrentTab = EDC1InventorySource::Player;

private:
	// 드래그 복제 Slot 배율 (원본 슬롯 크기 대비, 0.8 = 80%)
	static constexpr float DragSlotScale = 0.8f;

	// 드래그 복제 Slot
	UPROPERTY()
	TObjectPtr<UDC1InventorySlotWidget> DraggedSlotCopy = nullptr;

	// 드래그 복제 Slot의 현재 보간 위치 (뷰포트)
	FVector2D DraggedSlotCurrentPosition = FVector2D::ZeroVector;
};
