// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Pal/DC1PalStatusWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "UI/Bar/DC1StatusBar.h"

void UDC1PalStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Image_ElementIcon01->SetVisibility(ESlateVisibility::Collapsed);
}

void UDC1PalStatusWidget::SetPalInfo(const FDC1PalDisplayInfo& InDisplayInfo)
{
	Image_ElementIcon00->SetBrushFromTexture(InDisplayInfo.ElementIcon);

	Text_Name->SetText(InDisplayInfo.Name);
	Text_Level->SetText(FText::AsNumber(InDisplayInfo.Level));

	HPBar->SetPercent(InDisplayInfo.HealthPercent);
}

