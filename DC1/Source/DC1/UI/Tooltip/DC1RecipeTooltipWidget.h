// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Tooltip/DC1TooltipWidget.h"
#include "DC1RecipeTooltipWidget.generated.h"

class UDC1ItemHeaderWidget;
struct FDC1ItemRecipeRow;

UCLASS()
class DC1_API UDC1RecipeTooltipWidget : public UDC1TooltipWidget
{
	GENERATED_BODY()

public:
	void	SetRecipe(const FDC1ItemRecipeRow& InRecipe);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1ItemHeaderWidget>	ItemHeader = { nullptr };
};
