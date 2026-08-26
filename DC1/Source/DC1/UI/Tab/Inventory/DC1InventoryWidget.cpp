// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/Inventory/DC1InventoryWidget.h"
#include "UI/Tab/Inventory/DC1InventorySlotsWidget.h"
#include "ViewModel/Inventory/DC1InventoryViewModel.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"

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
	}
}

void UDC1InventoryWidget::NativeDestruct()
{
	if (CachedViewModel)
	{
		CachedViewModel->OnInventoryUpdated.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UDC1InventoryWidget::SetViewModel(UDC1InventoryViewModel* NewViewModel)
{
	// 기존 ViewModel 이벤트 구독 해제
	if (CachedViewModel)
	{
		CachedViewModel->OnInventoryUpdated.RemoveAll(this);
	}

	CachedViewModel = NewViewModel;

	if (CachedViewModel)
	{
		// ViewModel을 자식 위젯에 전달
		if (InventorySlotsWidget)
		{
			InventorySlotsWidget->SetInventoryViewModel(CachedViewModel);
		}

		// 이벤트 구독
		CachedViewModel->OnInventoryUpdated.AddUObject(this, &UDC1InventoryWidget::HandleInventoryUpdated);
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
