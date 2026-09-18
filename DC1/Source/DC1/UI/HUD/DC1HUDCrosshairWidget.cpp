// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/DC1HUDCrosshairWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Components/Image.h"
#include "Engine/LocalPlayer.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Player/DC1CrosshairViewModel.h"

void UDC1HUDCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
}

void UDC1HUDCrosshairWidget::NativeDestruct()
{
	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1HUDCrosshairWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		CrosshairViewModel = VMSubsystem->GetViewModel<UDC1CrosshairViewModel>();
		if (false == CrosshairViewModel.IsValid())
			return;

		CrosshairViewModel->OnVisibilityChanged.AddUObject(this, &ThisClass::HandleVisibilityChanged);
		CrosshairViewModel->OnFired.AddUObject(this, &ThisClass::HandleFired);

		HandleVisibilityChanged(CrosshairViewModel->IsVisible());
	}
}

void UDC1HUDCrosshairWidget::UnbindViewModel()
{
	if (false == CrosshairViewModel.IsValid())
		return;

	CrosshairViewModel->OnVisibilityChanged.RemoveAll(this);
	CrosshairViewModel->OnFired.RemoveAll(this);
	CrosshairViewModel.Reset();
}

void UDC1HUDCrosshairWidget::HandleVisibilityChanged(bool bInVisible)
{
	ESlateVisibility SlateVisible = bInVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
	SetVisibility(SlateVisible);
}

void UDC1HUDCrosshairWidget::HandleFired(float InStrength)
{
	const float Length = RecoilAnim->GetEndTime();
	//const float StartAt = Length * (1.f - FMath::Clamp(InStrength, 0.f, 1.f));

	// Temp
	const float StartAt = 0.f;
	
	PlayAnimation(RecoilAnim, StartAt);
}
