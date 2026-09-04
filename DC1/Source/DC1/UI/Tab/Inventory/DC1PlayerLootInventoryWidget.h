// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1PlayerLootInventoryWidget.generated.h"

class UDC1InventoryViewModel;
class UDC1InventorySlotsWidget;
class UDC1InventorySlotWidget;

class UTextBlock;
class UButton;

/**
 * 상자 개방 화면(DC1ChestLootScreenWidget)의 좌측에 배치되는 플레이어 인벤토리 패널
 * ESC 메뉴 전용인 UDC1InventoryWidget과는 별개의 경량 위젯 (상속하지 않음 - 용도가 다름)
 * UDC1ChestInventoryWidget과 동일한 UDC1InventoryViewModel 인스턴스를 공유하여 드래그 앤 드롭이 상호 연동됨
 */
UCLASS()
class DC1_API UDC1PlayerLootInventoryWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	UDC1PlayerLootInventoryWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	/** 상자 화면을 여는 쪽(DC1ChestLootScreenWidget)과 공유하는 ViewModel 주입 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetViewModel(UDC1InventoryViewModel* NewViewModel);

protected:
	void HandleDragEnded();

protected:
	// 상단 바 제목 텍스트 (상자 패널과 톤을 맞추기 위한 레이블, 상시 "Inventory" 고정 표시)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ChestNameText;

	// 분류 버튼, 상자 패널과 동일하게 자리만 예약 (기능은 추후 구현 예정)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SortButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1InventorySlotsWidget> PlayerSlotsWidget;

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
