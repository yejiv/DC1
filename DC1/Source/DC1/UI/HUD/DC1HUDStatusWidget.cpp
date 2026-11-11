// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/DC1HUDStatusWidget.h"

#include "Components/TextBlock.h"
#include "UI/Bar/DC1StatusBar.h"
#include "Engine/LocalPlayer.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Player/DC1PlayerStatusViewModel.h"

void UDC1HUDStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
}

void UDC1HUDStatusWidget::NativeDestruct()
{
	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1HUDStatusWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		StatusViewModel = VMSubsystem->GetViewModel<UDC1PlayerStatusViewModel>();
		if (false == StatusViewModel.IsValid())
			return;

		StatusViewModel->OnHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);

		HandleHealthChanged(StatusViewModel->GetHealth(), StatusViewModel->GetMaxHealth());
	}
}

void UDC1HUDStatusWidget::UnbindViewModel()
{
	if (false == StatusViewModel.IsValid())
		return;

	StatusViewModel->OnHealthChanged.RemoveAll(this);
	StatusViewModel.Reset();
}

void UDC1HUDStatusWidget::HandleHealthChanged(float Current, float Max)
{
	HealthBar->SetPercent(StatusViewModel->GetHealthPercent());
	// 정수 단위 표기
	Text_Health->SetText(FText::AsNumber(FMath::CeilToInt(Current)));
	Text_MaxHealth->SetText(FText::AsNumber(FMath::RoundToInt(Max)));
}
