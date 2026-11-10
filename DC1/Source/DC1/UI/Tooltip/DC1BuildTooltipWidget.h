// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Tooltip/DC1TooltipWidget.h"
#include "DC1BuildTooltipWidget.generated.h"

class UPanelWidget;
class UImage;
class UTexture2D;
struct FDC1BuildRow;

UCLASS()
class DC1_API UDC1BuildTooltipWidget : public UDC1TooltipWidget
{
	GENERATED_BODY()

public:
	void	SetBuild(const FDC1BuildRow& InRow);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>	InactiveBox = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>	WorkTypeBox = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>			Image_WorkType = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_RequiredWork = { nullptr };

	UPROPERTY(EditAnywhere, Category = "Tooltip|Build", meta = (Categories = "Work"))
	TMap<FGameplayTag, TSoftObjectPtr<UTexture2D>>	WorkTypeIconsByTag;
};
