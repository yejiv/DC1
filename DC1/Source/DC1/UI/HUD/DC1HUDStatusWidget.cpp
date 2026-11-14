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
		StatusViewModel->OnShieldChanged.AddUObject(this, &ThisClass::HandleShieldChanged);

		HandleHealthChanged(StatusViewModel->GetHealth(), StatusViewModel->GetMaxHealth());
		HandleShieldChanged(StatusViewModel->GetShield(), StatusViewModel->GetMaxShield());
	}
}

void UDC1HUDStatusWidget::UnbindViewModel()
{
	if (false == StatusViewModel.IsValid())
		return;

	StatusViewModel->OnHealthChanged.RemoveAll(this);
	StatusViewModel->OnShieldChanged.RemoveAll(this);
	StatusViewModel.Reset();
}

void UDC1HUDStatusWidget::HandleHealthChanged(float Current, float Max)
{
	HealthBar->SetPercent(StatusViewModel->GetHealthPercent());
	// 정수 단위 표기
	Text_Health->SetText(FText::AsNumber(FMath::CeilToInt(Current)));
	Text_MaxHealth->SetText(FText::AsNumber(FMath::RoundToInt(Max)));
}

void UDC1HUDStatusWidget::HandleShieldChanged(float Current, float Max)
{
	// 실드 장착 안함
	if (0.f >= Max)
	{
		// Verticla 위치 유지 Hidden
		ShieldBox->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	if (ESlateVisibility::Hidden == ShieldBox->GetVisibility())
	{
		ShieldBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	ShieldBar->SetPercent(StatusViewModel->GetShieldPercent());

	Text_Shield->SetText(FText::AsNumber(FMath::CeilToInt(Current)));
	Text_MaxShield->SetText(FText::AsNumber(FMath::CeilToInt(Max)));
}
