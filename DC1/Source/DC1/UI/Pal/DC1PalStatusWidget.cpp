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

	// 속성이 하나뿐인 팰은 두 번째 칸을 접는다
	Image_ElementIcon01->SetBrushFromTexture(InDisplayInfo.ElementIcon2);
	Image_ElementIcon01->SetVisibility((nullptr != InDisplayInfo.ElementIcon2) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	Text_Name->SetText(InDisplayInfo.Name);
	Text_Level->SetText(FText::AsNumber(InDisplayInfo.Level));

	// 다른 팰로 바뀐 거면 피격이 아니므로 연출 없이 바로 맞춘다
	if (DisplayedPalID == InDisplayInfo.PalID)
	{
		HPBar->SetPercent(InDisplayInfo.HealthPercent);
	}
	else
	{
		HPBar->SnapToPercent(InDisplayInfo.HealthPercent);
		DisplayedPalID = InDisplayInfo.PalID;
	}
}

