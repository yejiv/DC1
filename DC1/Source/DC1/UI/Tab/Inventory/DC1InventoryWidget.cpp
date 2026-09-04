// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/Inventory/DC1InventoryWidget.h"
#include "UI/Tab/Inventory/DC1InventorySlotsWidget.h"
#include "UI/Tab/Inventory/DC1InventorySlotWidget.h"
#include "ViewModel/Inventory/DC1InventoryViewModel.h"


#include "Data/DataTable/DC1DataTableTypes.h"

#include "Component/Inventory/DC1InventoryTypes.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetLayoutLibrary.h"

UDC1InventoryWidget::UDC1InventoryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDC1InventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (CachedViewModel)
	{
		HandleInventoryUpdated();
	}
}

void UDC1InventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CachedViewModel)
	{
		CachedViewModel->OnInventoryUpdated.AddUObject(this, &UDC1InventoryWidget::HandleInventoryUpdated);
		CachedViewModel->OnDragEnded.AddUObject(this, &UDC1InventoryWidget::HandleDragEnded);
	}
}

void UDC1InventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 드래그 중일 때 항상 마우스 위치를 서서히 따라다님 (버튼 상태와 무관)
	// DraggedSlotCopy는 이 위젯에서 드래그를 시작했을 때만 존재함
	if (IsValid(DraggedSlotCopy))
	{
		FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
		FVector2D TargetPos = MousePosition - FVector2D(DragSlotSize / 2.0f, DragSlotSize / 2.0f);

		DraggedSlotCurrentPosition = FMath::Vector2DInterpTo(DraggedSlotCurrentPosition, TargetPos, InDeltaTime, DragFollowSpeed);

		// GetMousePositionOnViewport와 짝을 이루는 뷰포트 좌표계이므로 DPI 재보정을 끔
		DraggedSlotCopy->SetPositionInViewport(DraggedSlotCurrentPosition, false);
	}
}

void UDC1InventoryWidget::NativeDestruct()
{
	if (CachedViewModel)
	{
		CachedViewModel->OnInventoryUpdated.RemoveAll(this);
		CachedViewModel->OnDragEnded.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UDC1InventoryWidget::SetViewModel(UDC1InventoryViewModel* NewViewModel)
{
	if (CachedViewModel)
	{
		CachedViewModel->OnInventoryUpdated.RemoveAll(this);
		CachedViewModel->OnDragEnded.RemoveAll(this);
	}

	CachedViewModel = NewViewModel;

	if (CachedViewModel)
	{
		if (InventorySlotsWidget)
		{
			InventorySlotsWidget->SetInventoryViewModel(CachedViewModel);
		}

		// 이벤트 구독
		CachedViewModel->OnInventoryUpdated.AddUObject(this, &UDC1InventoryWidget::HandleInventoryUpdated);
		CachedViewModel->OnDragEnded.AddUObject(this, &UDC1InventoryWidget::HandleDragEnded);
		HandleInventoryUpdated();
	}
}

void UDC1InventoryWidget::HandleInventoryUpdated()
{
	if (!CachedViewModel)
	{
		return;
	}

	if (InventoryTitle)
	{
		InventoryTitle->SetText(FText::FromString(TEXT("Inventory")));
	}
}

void UDC1InventoryWidget::HandleDragEnded()
{
	// 이 위젯에서 시작한 드래그였다면 시각 복제본을 정리 (다른 위젯에서 시작한 드래그라면 DraggedSlotCopy가 nullptr이라 무시됨)
	if (IsValid(DraggedSlotCopy))
	{
		DraggedSlotCopy->RemoveFromParent();
		DraggedSlotCopy = nullptr;
	}
}

FReply UDC1InventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (!IsValid(InventorySlotsWidget) || !IsValid(CachedViewModel))
	{
		return FReply::Unhandled();
	}

	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	int32 ClickedSlotIndex = InventorySlotsWidget->CalculateSlotIndex(InMouseEvent.GetScreenSpacePosition());
	int32 TotalSlots = CachedViewModel->GetSlotCount();
	if (ClickedSlotIndex < 0 || ClickedSlotIndex >= TotalSlots)
	{
		return FReply::Unhandled();
	}

	if (!CachedViewModel->IsDragging())
	{
		const FDC1InventorySlotInfo& InventorySlotInfo = CachedViewModel->GetSlot(ClickedSlotIndex);
		if (InventorySlotInfo.IsEmpty())
		{
			return FReply::Unhandled();
		}

		// 키 조합에 따른 개수 선택
		int32 DragStackCount = InventorySlotInfo.StackCount;
		if (InMouseEvent.IsControlDown())
		{
			DragStackCount = 1;
		}
		else if (InMouseEvent.IsShiftDown())
		{
			DragStackCount = FMath::Max(1, InventorySlotInfo.StackCount / 2);
		}

		if (!CachedViewModel->GetItemData(InventorySlotInfo.ItemID))
		{
			return FReply::Unhandled();
		}

		CachedViewModel->BeginDrag(false, ClickedSlotIndex, DragStackCount);

		// Slot 위젯 복제
		if (!DraggedSlotCopy && IsValid(InventorySlotsWidget))
		{
			DraggedSlotCopy = CreateWidget<UDC1InventorySlotWidget>(GetOwningPlayer(), DragSlotWidgetClass);
			if (IsValid(DraggedSlotCopy))
			{
				// 복제본 초기화: 크기는 80x80 (기존 100x100의 80%)
				DraggedSlotCopy->InitSlot(ClickedSlotIndex, DragSlotSize, false);
				DraggedSlotCopy->SetSlotData(CachedViewModel);

				// Ctrl/Shift로 선택한 부분 개수를 표시 (원본 슬롯의 전체 개수가 아님)
				DraggedSlotCopy->OverrideDisplayCount(DragStackCount);

				// 무게 텍스트 비활성화
				DraggedSlotCopy->SetWeightVisibility(false);

				// 드래그 복제본 스타일 설정
				DraggedSlotCopy->InitializeDragCopy();

				// 뷰포트에 추가
				DraggedSlotCopy->AddToViewport(999);

				// 초기 위치를 마우스 위치(복제본 중심 기준)로 설정하여 첫 프레임에 순간이동하지 않도록 함
				FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
				DraggedSlotCurrentPosition = MousePos - FVector2D(DragSlotSize / 2.0f, DragSlotSize / 2.0f);
				DraggedSlotCopy->SetPositionInViewport(DraggedSlotCurrentPosition, false);
			}
		}

		return FReply::Handled();
	}
	else
	{
		// 드래그 중 슬롯 클릭: 드롭 처리
		// 드래그 시작이 상자였다면 ViewModel이 플레이어<->상자 이전으로 자동 라우팅함
		CachedViewModel->RequestDrop(false, ClickedSlotIndex);

		return FReply::Handled();
	}
}
