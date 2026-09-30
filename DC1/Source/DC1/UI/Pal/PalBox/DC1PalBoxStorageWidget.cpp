// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Pal/PalBox/DC1PalBoxStorageWidget.h"

#include "UI/Pal/PalBox/DC1PalBoxStorageSlotWidget.h"

#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"

void UDC1PalBoxStorageWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (nullptr != Text_Title && false == Title.IsEmpty())
	{
		Text_Title->SetText(Title);
	}
}

void UDC1PalBoxStorageWidget::CreateSlots(int32 InCapacity)
{
	if (SlotWidgets.Num() == InCapacity || nullptr == SlotWidgetClass)
	{
		return;
	}

	SlotGrid->ClearChildren();
	SlotWidgets.Reset();

	for (int32 Index = 0; Index < InCapacity; ++Index)
	{
		UDC1PalBoxStorageSlotWidget* SlotWidget = CreateWidget<UDC1PalBoxStorageSlotWidget>(this, SlotWidgetClass);
		if (nullptr == SlotWidget)
		{
			continue;
		}

		SlotWidget->SetSlotIndex(Index);
		SlotWidget->OnSlotHovered.AddUObject(this, &ThisClass::HandleSlotHovered);

		SlotGrid->AddChildToUniformGrid(SlotWidget, Index / Columns, Index % Columns);
		SlotWidgets.Add(SlotWidget);
	}
}

void UDC1PalBoxStorageWidget::SetSlotInfo(int32 InSlotIndex, const FDC1PalBoxSlotInfo& InSlotInfo)
{
	if (true == SlotWidgets.IsValidIndex(InSlotIndex))
	{
		SlotWidgets[InSlotIndex]->SetSlotInfo(InSlotInfo);
	}
}

void UDC1PalBoxStorageWidget::SetEmpty(int32 InSlotIndex)
{
	if (true == SlotWidgets.IsValidIndex(InSlotIndex))
	{
		SlotWidgets[InSlotIndex]->SetEmpty();
	}
}

void UDC1PalBoxStorageWidget::HandleSlotHovered(int32 InSlotIndex)
{
	OnSlotHovered.Broadcast(InSlotIndex);
}
