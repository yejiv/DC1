// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1ChestInventoryWidget.generated.h"

class UDC1InventoryViewModel;
class UDC1InventorySlotsWidget;
class UDC1InventorySlotWidget;
class UDC1ChestInventoryComponent;

class UTextBlock;
class UButton;

/**
 * 상자 인벤토리 표시용 위젯
 * 상단에 상자 이름 + 분류/한번에 옮기기 버튼(자리만 예약, 기능은 추후 구현), 하단에 UDC1InventorySlotsWidget으로 상자 내용물 표시
 * 플레이어 인벤토리 위젯(UDC1InventoryWidget)과 동일한 UDC1InventoryViewModel 인스턴스를 공유하여 드래그 앤 드롭이 상호 연동됨
 */
UCLASS()
class DC1_API UDC1ChestInventoryWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	UDC1ChestInventoryWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	/** 플레이어 인벤토리 위젯과 공유하는 ViewModel 주입 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetViewModel(UDC1InventoryViewModel* NewViewModel);

	/** 표시할 상자를 지정하여 상자 UI를 연다 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenChest(UDC1ChestInventoryComponent* InChestComp);

protected:
	void HandleChestInventoryUpdated();
	void HandleDragEnded();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ChestNameText;

	// 분류 버튼, 우측 끝에 배치. 기능은 추후 구현 예정 - 자리만 예약
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SortButton;

	// 한번에 옮기기 버튼, 분류 버튼 좌측에 배치. 기능은 추후 구현 예정 - 자리만 예약
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MoveAllButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotsWidget> ChestSlotsWidget;

	UPROPERTY()
	TObjectPtr<UDC1InventoryViewModel> CachedViewModel;

	UPROPERTY()
	TObjectPtr<UDC1ChestInventoryComponent> CachedChestComp;

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
