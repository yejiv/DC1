// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Pal/DC1PalViewModel.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/Pal/DC1PalPartyComponent.h"

void UDC1PalViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	if (nullptr == InPlayerState)
		return;

	PalPartyComponent = InPlayerState->GetPalPartyComponent();

	if (false == PalPartyComponent.IsValid())
		return;

	Capacity = PalPartyComponent->GetCapacity();
	PrevSlot = PalPartyComponent->GetSelectedSlot();
	PalPartyComponent->OnPalSlotTranslated.AddUObject(this, &ThisClass::HandleSelectedSlotChanged);
}

void UDC1PalViewModel::ClearSource()
{
	if (false == PalPartyComponent.IsValid())
		return;

	PalPartyComponent->OnPalSlotTranslated.RemoveAll(this);
	PalPartyComponent.Reset();
}

void UDC1PalViewModel::HandleSelectedSlotChanged()
{
	if (false == PalPartyComponent.IsValid())
		return;

	int32 SelectedIndex = PalPartyComponent->GetSelectedSlot();

	int32 Move = (PrevSlot - SelectedIndex + Capacity) % Capacity;

	PrevSlot = SelectedIndex;

	EDC1PalSlotShift Shift = (Move == 1) ? EDC1PalSlotShift::Prev : ((Move == 0) ? EDC1PalSlotShift::None : EDC1PalSlotShift::Next);
	OnSlotShifted.Broadcast(Shift);
}