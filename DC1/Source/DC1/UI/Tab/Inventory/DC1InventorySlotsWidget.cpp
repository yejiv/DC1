// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/Inventory/DC1InventorySlotsWidget.h"
#include "UI/Tab/Inventory/DC1InventorySlotWidget.h"
#include "ViewModel/Inventory/DC1InventoryViewModel.h"
#include "Components/ScrollBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

UDC1InventorySlotsWidget::UDC1InventorySlotsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDC1InventorySlotsWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (SlotWidgets.Num() == 0)
	{
		CreateSlotWidgets(DefaultSlotCapacity);
	}
}

void UDC1InventorySlotsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CachedViewModel)
	{
		if (bIsChestSlots)
		{
			CachedViewModel->OnChestSlotUIUpdated.AddUObject(this, &UDC1InventorySlotsWidget::HandleSlotUpdated);
			CachedViewModel->OnChestInventoryUpdated.AddUObject(this, &UDC1InventorySlotsWidget::HandleInventoryUpdated);
		}
		else
		{
			// 부분 갱신 이벤트 구독 (성능 최적화)
			CachedViewModel->OnSlotUIUpdated.AddUObject(this, &UDC1InventorySlotsWidget::HandleSlotUpdated);
			// 레거시 전체 갱신 이벤트도 유지 (필요시)
			CachedViewModel->OnInventoryUpdated.AddUObject(this, &UDC1InventorySlotsWidget::HandleInventoryUpdated);
		}
		RefreshAllSlots();
	}
}

void UDC1InventorySlotsWidget::NativeDestruct()
{
	if (CachedViewModel)
	{
		CachedViewModel->OnSlotUIUpdated.RemoveAll(this);
		CachedViewModel->OnInventoryUpdated.RemoveAll(this);
		CachedViewModel->OnChestSlotUIUpdated.RemoveAll(this);
		CachedViewModel->OnChestInventoryUpdated.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UDC1InventorySlotsWidget::SetInventoryViewModel(UDC1InventoryViewModel* InViewModel)
{
	// 기존 ViewModel 이벤트 구독 해제
	if (CachedViewModel)
	{
		CachedViewModel->OnSlotUIUpdated.RemoveAll(this);
		CachedViewModel->OnInventoryUpdated.RemoveAll(this);
		CachedViewModel->OnChestSlotUIUpdated.RemoveAll(this);
		CachedViewModel->OnChestInventoryUpdated.RemoveAll(this);
	}

	CachedViewModel = InViewModel;

	if (CachedViewModel)
	{
		if (bIsChestSlots)
		{
			CachedViewModel->OnChestSlotUIUpdated.AddUObject(this, &UDC1InventorySlotsWidget::HandleSlotUpdated);
			CachedViewModel->OnChestInventoryUpdated.AddUObject(this, &UDC1InventorySlotsWidget::HandleInventoryUpdated);
		}
		else
		{
			// 부분 갱신 이벤트 구독 (성능 최적화)
			CachedViewModel->OnSlotUIUpdated.AddUObject(this, &UDC1InventorySlotsWidget::HandleSlotUpdated);
			// 레거시 전체 갱신 이벤트도 유지 (필요시)
			CachedViewModel->OnInventoryUpdated.AddUObject(this, &UDC1InventorySlotsWidget::HandleInventoryUpdated);
		}
		RefreshAllSlots();
	}
}

void UDC1InventorySlotsWidget::RefreshSlotCapacity(int32 NewTotalSlots)
{
	CreateSlotWidgets(NewTotalSlots);
}

void UDC1InventorySlotsWidget::RefreshAllSlots()
{
	if (!CachedViewModel)
	{
		return;
	}

	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		UpdateSlotDisplay(i);
	}
}

int32 UDC1InventorySlotsWidget::CalculateSlotIndex(FVector2D ScreenSpaceMousePos)
{
	if (!SlotGrid)
	{
		return -1;
	}

	//마우스의 화면 절대 좌표를 Grid 패널 기준의 로컬 좌표로 변환
	const FGeometry& GridGeometry = SlotGrid->GetCachedGeometry();
	FVector2D MouseWidgetPos = GridGeometry.AbsoluteToLocal(ScreenSpaceMousePos);

	FMargin GridPadding = SlotGrid->SlotPadding;

	const float CellStrideX = GridPadding.Left + SlotSize + GridPadding.Right;
	const float CellStrideY = GridPadding.Top + SlotSize + GridPadding.Bottom;

	int32 SlotColumn = FMath::FloorToInt(MouseWidgetPos.X / CellStrideX);
	int32 SlotRow = FMath::FloorToInt(MouseWidgetPos.Y / CellStrideY);

	// 여백클릭 검사: 셀 내부에서의 상대 좌표(나머지)를 구함
	float OffsetX = FMath::Fmod(MouseWidgetPos.X, CellStrideX);
	float OffsetY = FMath::Fmod(MouseWidgetPos.Y, CellStrideY);

	// 마우스가 실제 슬롯(이미지) 영역을 벗어나 패딩(빈 공간)에 위치했다면 무시
	if (OffsetX < GridPadding.Left || OffsetX >(GridPadding.Left + SlotSize) ||
		OffsetY < GridPadding.Top || OffsetY >(GridPadding.Top + SlotSize))
	{
		return -1;
	}

	if (SlotColumn < 0 || SlotColumn >= InventoryColumns)
	{
		return -1;
	}

	int32 SourceSlotIndex = (SlotRow * InventoryColumns) + SlotColumn;

	return SourceSlotIndex;
}

void UDC1InventorySlotsWidget::CreateSlotWidgets(int32 TotalSlots)
{
	if (!SlotGrid)
	{
		UE_LOG(LogTemp, Error, TEXT("[UDC1InventorySlotsWidget] SlotGrid is NULL"));
		return;
	}

	if (!SlotWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UDC1InventorySlotsWidget] SlotWidgetClass is not assigned in Blueprint Details."));
		return;
	}

	SlotGrid->ClearChildren();
	SlotWidgets.Empty();
	SlotWidgets.Reserve(TotalSlots);

	for (int32 Index = 0; Index < TotalSlots; ++Index)
	{
		UDC1InventorySlotWidget* NewSlotWidget = CreateWidget<UDC1InventorySlotWidget>(this, SlotWidgetClass);
		if (NewSlotWidget)
		{
			const int32 Row = Index / InventoryColumns;
			const int32 Column = Index % InventoryColumns;

			NewSlotWidget->InitSlot(Index, SlotSize, bIsChestSlots);

			UUniformGridSlot* GridSlot = SlotGrid->AddChildToUniformGrid(NewSlotWidget, Row, Column);
			if (GridSlot)
			{
				GridSlot->SetHorizontalAlignment(HAlign_Center);
				GridSlot->SetVerticalAlignment(VAlign_Center);
			}

			SlotWidgets.Add(NewSlotWidget);
		}
	}
}

void UDC1InventorySlotsWidget::UpdateSlotDisplay(int32 SlotIndex)
{
	if (!CachedViewModel || !SlotWidgets.IsValidIndex(SlotIndex))
	{
		return;
	}

	UDC1InventorySlotWidget* SlotWidget = SlotWidgets[SlotIndex];
	if (!SlotWidget)
	{
		return;
	}

	SlotWidget->SetSlotData(CachedViewModel);
}

void UDC1InventorySlotsWidget::HandleInventoryUpdated()
{
	// 레거시: 전체 슬롯 갱신 (필요시 호출)
	RefreshAllSlots();
}

void UDC1InventorySlotsWidget::HandleSlotUpdated(int32 SlotIndex)
{
	// 부분 갱신: 특정 슬롯만 업데이트 (성능 최적화)
	UpdateSlotDisplay(SlotIndex);
}

int32 UDC1InventorySlotsWidget::GetSlotIndexFromPosition(FVector2D MousePosition) const
{
	if (!SlotGrid)
	{
		return -1;
	}

	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		UDC1InventorySlotWidget* SlotWidget = SlotWidgets[i];
		if (!IsValid(SlotWidget))
		{
			continue;
		}

		FVector2D InventorySlotSize = SlotWidget->GetTickSpaceGeometry().GetLocalSize();

		FVector2D SlotAbsolutePosition = SlotWidget->GetTickSpaceGeometry().GetAbsolutePosition();

		GEngine->AddOnScreenDebugMessage(
			-1,
			2.f,
			FColor::Blue,
			FString::Printf(TEXT("절대 위치 : %f / %f"), SlotAbsolutePosition.X, SlotAbsolutePosition.Y)
		);

		FVector2D RelativeMousePos = MousePosition - SlotAbsolutePosition;

		GEngine->AddOnScreenDebugMessage(
			-1,
			2.f,
			FColor::Blue,
			FString::Printf(TEXT("상대 위치 : %f / %f"), RelativeMousePos.X, RelativeMousePos.Y)
		);

		// 마우스가 슬롯의 가로/세로 영역 안에 있는지 확인
		if (RelativeMousePos.X >= 0 && RelativeMousePos.X <= InventorySlotSize.X &&
			RelativeMousePos.Y >= 0 && RelativeMousePos.Y <= InventorySlotSize.Y)
		{


			return i;
		}
	}

	return -1;
}
