// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Pal/PalBox/DC1PalBoxWorkEntryWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDC1PalBoxWorkEntryWidget::SetWorkInfo(const FDC1PalBoxWorkInfo& InWorkInfo)
{
	const bool bSuitable = (0 < InWorkInfo.Level);

	Image_Icon->SetBrushFromTexture(InWorkInfo.Icon);

	Text_Level->SetText(FText::AsNumber(InWorkInfo.Level));
	Text_Level->SetVisibility(bSuitable ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	SetRenderOpacity(bSuitable ? 1.f : DisabledOpacity);
}
