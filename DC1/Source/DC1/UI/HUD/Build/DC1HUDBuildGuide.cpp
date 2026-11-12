// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/Build/DC1HUDBuildGuide.h"

#include "Components/PanelWidget.h"
#include "Components/WidgetSwitcher.h"

void UDC1HUDBuildGuide::SetMode(EDC1BuildMode InMode)
{
	if (false == ::IsValid(GuideSwitcher))
	{
		return;
	}

	UWidget* ActiveGuide = (EDC1BuildMode::Dismantle == InMode) ? DismantleGuide.Get() : PlacementGuide.Get();
	if (nullptr != ActiveGuide)
	{
		GuideSwitcher->SetActiveWidget(ActiveGuide);
	}
}
