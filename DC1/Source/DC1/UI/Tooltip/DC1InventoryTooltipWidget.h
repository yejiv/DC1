// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Tooltip/DC1TooltipWidget.h"
#include "DC1InventoryTooltipWidget.generated.h"

class UPanelWidget;
class UDC1ItemHeaderWidget;

UCLASS()
class DC1_API UDC1InventoryTooltipWidget : public UDC1TooltipWidget
{
	GENERATED_BODY()

public:
	void	SetInventoryItem(FName InItemID, int32 InStackCount);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1ItemHeaderWidget>	ItemHeader = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>			WeightBox = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>				Text_Weight = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>			CountBox = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>				Text_Count = { nullptr };
};
