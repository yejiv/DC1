// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/DC1StaminaWidget.h"

#include "Components/Image.h"
#include "UI/Bar/DC1StatusBar.h"
#include "Engine/LocalPlayer.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Player/DC1PlayerStatusViewModel.h"

void UDC1StaminaWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();

	//StaminaBar->SetVisibility(ESlateVisibility::Collapsed);
}

void UDC1StaminaWidget::NativeDestruct()
{
	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1StaminaWidget::BindViewModel()
{
	if (ULocalPlayer* Player = GetOwningLocalPlayer())
	{
		if (UDC1ViewModelSubsystem* VMSubsystem = Player->GetSubsystem<UDC1ViewModelSubsystem>())
		{
			StatusViewModel = VMSubsystem->GetViewModel<UDC1PlayerStatusViewModel>();
			if (false == StatusViewModel.IsValid())
				return;

			StatusViewModel->OnStaminaChanged.AddUObject(this, &ThisClass::HandleStaminaChanged);

			HandleStaminaChanged(StatusViewModel->GetStamina(), StatusViewModel->GetMaxStamina());
		}
	}
}

void UDC1StaminaWidget::UnbindViewModel()
{
	if (false == StatusViewModel.IsValid())
		return;

	StatusViewModel->OnStaminaChanged.RemoveAll(this);
	StatusViewModel.Reset();
}

void UDC1StaminaWidget::HandleStaminaChanged(float Current, float Max)
{
	StaminaBar->SetPercent(StatusViewModel->GetStaminaPercent());

	// 꽉 차면 Visiblity Off
	if (Current >= Max)
	{
		StaminaBar->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		StaminaBar->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}