// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/DC1HUDDownedWidget.h"

#include "UI/Bar/DC1StatusBar.h"
#include "UI/World/Interact/DC1InteractPromptRowWidget.h"
#include "Engine/LocalPlayer.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Player/DC1PlayerStatusViewModel.h"

void UDC1HUDDownedWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();

	SetVisibility(ESlateVisibility::Collapsed);
	HandleDownedStateChanged(StatusViewModel->IsDowned());

	GiveUpOption.InputType = EDC1InteractInputType::Press;
	GiveUpOption.Key = EDC1InteractKey::F;
	GiveUpOption.DisplayText = FText::FromString("포기");
	
	GiveUpPrompt->SetOption(GiveUpOption);
}

void UDC1HUDDownedWidget::NativeDestruct()
{
	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1HUDDownedWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		StatusViewModel = VMSubsystem->GetViewModel<UDC1PlayerStatusViewModel>();
		if (false == StatusViewModel.IsValid())
			return;

		StatusViewModel->OnDownGaugeChanged.AddUObject(this, &ThisClass::HandleDownedGaugeChanged);
		StatusViewModel->OnDownStateChanged.AddUObject(this, &ThisClass::HandleDownedStateChanged);

		HandleDownedGaugeChanged(StatusViewModel->GetDownGauge(), StatusViewModel->GetMaxDownGauge());
	}
}

void UDC1HUDDownedWidget::UnbindViewModel()
{
	if (false == StatusViewModel.IsValid())
		return;

	StatusViewModel->OnDownGaugeChanged.RemoveAll(this);
	StatusViewModel->OnDownStateChanged.RemoveAll(this);
	StatusViewModel.Reset();
}

void UDC1HUDDownedWidget::HandleDownedStateChanged(bool InIsDowned)
{
	SetVisibility(InIsDowned ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UDC1HUDDownedWidget::HandleDownedGaugeChanged(float Current, float Max)
{
	DownedGaugeBar->SetPercent(StatusViewModel->GetDownGaugePercent());
}
