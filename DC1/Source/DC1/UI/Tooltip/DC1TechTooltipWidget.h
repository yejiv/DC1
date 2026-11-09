// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Tooltip/DC1TooltipWidget.h"
#include "DC1TechTooltipWidget.generated.h"

class UImage;
struct FDC1TechRow;
struct FDC1ItemCost;

UCLASS()
class DC1_API UDC1TechTooltipWidget : public UDC1TooltipWidget
{
	GENERATED_BODY()

public:
	void	SetTech(const FDC1TechRow& InRow, const FText& InDescription, const TArray<FDC1ItemCost>& InCosts);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		Image_Icon = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>	Text_TechPoint = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>	Text_Category = { nullptr };
};
