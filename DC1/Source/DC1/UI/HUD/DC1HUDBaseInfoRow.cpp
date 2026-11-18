// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/DC1HUDBaseInfoRow.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDC1HUDBaseInfoRow::SetRow(UTexture2D* InPalIcon, const FText& InConditionText, bool InWorking)
{
	Image_PalIcon->SetBrushFromTexture(InPalIcon);
	Text_Condition->SetText(InConditionText);
	Image_Work->SetVisibility(InWorking ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}
