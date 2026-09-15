// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/DC1HUDPalWidget.h"

#include "Engine/LocalPlayer.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Pal/DC1PalViewModel.h"

void UDC1HUDPalWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
}

void UDC1HUDPalWidget::NativeDestruct()
{
	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1HUDPalWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		PalViewModel = VMSubsystem->GetViewModel<UDC1PalViewModel>();
		if (false == PalViewModel.IsValid())
			return;

		PalViewModel->OnSlotShifted.AddUObject(this, &ThisClass::HandleSlotShifted);
		PalViewModel->OnInfoChanged.AddUObject(this, &ThisClass::HandleInfoChanged);

		HandleInfoChanged();
	}
}

void UDC1HUDPalWidget::UnbindViewModel()
{
	if (false == PalViewModel.IsValid())
		return;

	PalViewModel->OnSlotShifted.RemoveAll(this);
	PalViewModel->OnInfoChanged.RemoveAll(this);
	PalViewModel.Reset();
}

void UDC1HUDPalWidget::HandleSlotShifted(EDC1PalSlotShift InShift)
{

}

void UDC1HUDPalWidget::HandleInfoChanged()
{

}
