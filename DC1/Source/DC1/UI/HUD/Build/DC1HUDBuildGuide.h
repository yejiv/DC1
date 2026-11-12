// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Define/DC1Enums.h"
#include "DC1HUDBuildGuide.generated.h"

class UPanelWidget;
class UWidgetSwitcher;

UCLASS()
class DC1_API UDC1HUDBuildGuide : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void SetMode(EDC1BuildMode InMode);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher>		GuideSwitcher = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPanelWidget>		PlacementGuide = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPanelWidget>		DismantleGuide = { nullptr };
};
