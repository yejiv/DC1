// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/Inventory/DC1InventorySlotsWidget.h"
#include "Components/ScrollBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "UI/Tab/Inventory/DC1InventorySlotWidget.h"
#include "UI/Tab/Inventory/DC1EquipmentWidget.h"
#include "ViewModel/Inventory/DC1InventoryViewModel.h"


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


}

void UDC1InventorySlotsWidget::NativeDestruct()
{
	if (CachedViewModel)
	{
		// ViewModel 이벤트 해제 
		// CachedViewModel->OnInventoryChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UDC1InventorySlotsWidget::SetViewModel(UDC1InventoryViewModel* NewViewModel)
{
	if (CachedViewModel == NewViewModel)
	{
		return;
	}

	if (CachedViewModel)
	{
		// CachedViewModel->OnInventoryChanged.RemoveAll(this);
	}

	CachedViewModel = NewViewModel;

	if (CachedViewModel)
	{
		// ViewModel 이벤트 바인딩
		// CachedViewModel->OnInventoryChanged.AddDynamic(this, &ThisClass::HandleInventoryUpdated);

		// ViewModel의 현재 총 용량에 맞춰 슬롯 생성 (예: 기본 35개, 확장 시 N개)
		// int32 TotalCapacity = CachedViewModel->GetTotalCapacity();
		// RefreshSlotCapacity(TotalCapacity);

		HandleInventoryUpdated();
	}
}

void UDC1InventorySlotsWidget::RefreshSlotCapacity(int32 NewTotalSlots)
{
	// 슬롯 개수가 늘어나거나 변경되었을 때
	CreateSlotWidgets(NewTotalSlots);
	HandleInventoryUpdated();
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

	// 기존 그리드 및 위젯 배열 초기화
	SlotGrid->ClearChildren();
	SlotWidgets.Empty();
	SlotWidgets.Reserve(TotalSlots);

	// 무조건 가로 5개 고정
	for (int32 Index = 0; Index < TotalSlots; ++Index)
	{
		UDC1InventorySlotWidget* NewSlotWidget = CreateWidget<UDC1InventorySlotWidget>(this, SlotWidgetClass);
		if (NewSlotWidget)
		{
			const int32 Row = Index / InventoryColumns;
			const int32 Column = Index % InventoryColumns; 

			UUniformGridSlot* GridSlot = SlotGrid->AddChildToUniformGrid(NewSlotWidget, Row, Column);
			if (GridSlot)
			{
				//GridSlot->SetMargin(SlotMargin);
				GridSlot->SetHorizontalAlignment(HAlign_Center);
				GridSlot->SetVerticalAlignment(VAlign_Center);
			}

			// NewSlotWidget->SetSlotIndex(Index);

			SlotWidgets.Add(NewSlotWidget);
		}
	}
}

void UDC1InventorySlotsWidget::HandleInventoryUpdated()
{
	if (!CachedViewModel)
	{
		return;
	}

	// [아이템 획득 및 슬롯 자동 확장 로직 예시]
	/*
	int32 RequiredCapacity = CachedViewModel->GetTotalCapacity();

	// 필요한 용량이 현재 UI 슬롯 개수보다 많으면 자동으로 5개 단위/필요 개수만큼 확장
	if (SlotWidgets.Num() < RequiredCapacity)
	{
		RefreshSlotCapacity(RequiredCapacity);
		return;
	}

	// 각 슬롯 데이터 UI 갱신
	const TArray<FDC1ItemSlotData>& ItemList = CachedViewModel->GetItemSlots();
	const int32 LoopCount = FMath::Min(SlotWidgets.Num(), ItemList.Num());

	for (int32 Index = 0; Index < LoopCount; ++Index)
	{
		if (SlotWidgets[Index])
		{
			SlotWidgets[Index]->UpdateSlot(ItemList[Index]);
		}
	}
	*/
}