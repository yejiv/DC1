// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/PalInfo/DC1PalDetailWidget.h"

#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Pal/DC1PalBoxViewModel.h"

void UDC1PalDetailWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
}

void UDC1PalDetailWidget::NativeDestruct()
{
	UnBindViewModel();

	Super::NativeDestruct();
}

void UDC1PalDetailWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		CachedViewModel = VMSubsystem->GetViewModel<UDC1PalBoxViewModel>();
	}
}

void UDC1PalDetailWidget::UnBindViewModel()
{
	CachedViewModel.Reset();
}
