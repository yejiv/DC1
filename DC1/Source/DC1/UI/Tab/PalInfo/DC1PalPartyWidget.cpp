// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/PalInfo/DC1PalPartyWidget.h"

#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Pal/DC1PalBoxViewModel.h"
#include "ViewModel/Pal/DC1PalViewModel.h"

#include "UI/Pal/PalBox/DC1PalBoxSlotWidget.h"

#include "Components/DynamicEntryBox.h"

void UDC1PalPartyWidget::RefreshPalPartySlots()
{
	const TArray<UUserWidget*>& SlotWidgets = EntryBox_Slots->GetAllEntries();

	int32 SlotNum = SlotWidgets.Num();

	for (int32 i = 0; i < SlotNum; ++i)
	{
		if (UDC1PalBoxSlotWidget* SlotWidget = Cast<UDC1PalBoxSlotWidget>(SlotWidgets[i]))
		{
			RefreshPalInfo(SlotWidget, i);
		}
	}
}

void UDC1PalPartyWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BindViewModel();

	if (false == CachedPalBoxViewModel.IsValid())
		return;

	int32 Capacity = CachedPalBoxViewModel->GetCapacity();
	for (int32 i = 0; i < Capacity; ++i)
	{
		if (UDC1PalBoxSlotWidget* SlotWidget = EntryBox_Slots->CreateEntry<UDC1PalBoxSlotWidget>())
		{
			SlotWidget->SetSlotIndex(i);
		}
	}
}

void UDC1PalPartyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const TArray<UUserWidget*>& SlotWidgets = EntryBox_Slots->GetAllEntries();
	for (int32 i = 0; i < SlotWidgets.Num(); ++i)
	{
		if (UDC1PalBoxSlotWidget* SlotWidget = Cast<UDC1PalBoxSlotWidget>(SlotWidgets[i]))
		{
			SlotWidget->OnSlotHovered.AddUObject(this, &ThisClass::HandleSlotHovered);

			// 클릭 델리게이트는 인자가 없어 슬롯 번호를 같이 묶어둠
			SlotWidget->OnClickedDelegate.AddUObject(this, &ThisClass::HandleSlotClicked, i);
		}
	}
}

void UDC1PalPartyWidget::NativeDestruct()
{
	const TArray<UUserWidget*>& SlotWidgets = EntryBox_Slots->GetAllEntries();
	for (auto& Widget : SlotWidgets)
	{
		if (UDC1PalBoxSlotWidget* SlotWidget = Cast<UDC1PalBoxSlotWidget>(Widget))
		{
			SlotWidget->OnSlotHovered.RemoveAll(this);
			SlotWidget->OnClickedDelegate.RemoveAll(this);
		}
	}

	Super::NativeDestruct();
}

void UDC1PalPartyWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		CachedPalBoxViewModel = VMSubsystem->GetViewModel<UDC1PalBoxViewModel>();
		CachedPalViewModel = VMSubsystem->GetViewModel<UDC1PalViewModel>();
	}
}

void UDC1PalPartyWidget::RefreshPalInfo(UDC1PalBoxSlotWidget* InSlot, int32 SlotIndex)
{
	FDC1PalBoxSlotInfo SlotInfo = {};
	if (false == CachedPalBoxViewModel->GetSlotInfo(EDC1PalContainer::Party, SlotIndex, SlotInfo))
	{
		InSlot->SetEmpty();
	}
	else
	{
		InSlot->SetSlotInfo(SlotInfo);
	}
}

void UDC1PalPartyWidget::HandleSlotHovered(int32 SlotIndex)
{
	if (false == CachedPalViewModel.IsValid())
		return;

	const FGameplayTag& SpeciesTag = CachedPalViewModel->GetSpeciesTag(SlotIndex);

	OnChangedFocusingPal.ExecuteIfBound(SpeciesTag);
}

void UDC1PalPartyWidget::HandleSlotClicked(int32 SlotIndex)
{
	if (false == CachedPalViewModel.IsValid())
		return;

	const FGameplayTag& SpeciesTag = CachedPalViewModel->GetSpeciesTag(SlotIndex);

	OnSelectedPal.ExecuteIfBound(SlotIndex, SpeciesTag);
}
