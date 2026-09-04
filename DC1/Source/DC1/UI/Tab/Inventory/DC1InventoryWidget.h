// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1InventoryWidget.generated.h"

class UDC1InventoryViewModel;
class UDC1InventorySlotsWidget;
class UDC1InventorySlotWidget;

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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetViewModel(UDC1InventoryViewModel* NewViewModel);

protected:
	void HandleInventoryUpdated();
	void HandleDragEnded();

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

	UPROPERTY(EditAnywhere, Category = "Inventory|Drag")
	TSubclassOf<UDC1InventorySlotWidget> DragSlotWidgetClass;

	/** 드래그 복제본이 마우스를 따라가는 보간 속도 (클수록 빠르게 따라붙음) */
	UPROPERTY(EditAnywhere, Category = "Inventory|Drag")
	float DragFollowSpeed = 15.0f;

private:
	// 드래그 복제 Slot 크기 (기존 100x100의 80%)
	static constexpr float DragSlotSize = 80.0f;

	// 드래그 복제 Slot 위젯 (드래그 세션 자체는 ViewModel이 공유 상태로 관리)
	UPROPERTY()
	TObjectPtr<UDC1InventorySlotWidget> DraggedSlotCopy = nullptr;

	// 드래그 복제 Slot의 현재 보간 위치 (뷰포트 좌표계)
	FVector2D DraggedSlotCurrentPosition = FVector2D::ZeroVector;
};
