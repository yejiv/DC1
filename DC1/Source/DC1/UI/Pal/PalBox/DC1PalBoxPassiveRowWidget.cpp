// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Pal/PalBox/DC1PalBoxPassiveRowWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UDC1PalBoxPassiveRowWidget::SetPassiveInfo(const FDC1PalBoxPassiveInfo& InPassiveInfo)
{
	Text_Name->SetText(InPassiveInfo.Name);

	if (nullptr == Image_Rank)
	{
		return;
	}

	Image_Rank->SetBrushFromTexture(InPassiveInfo.RankIcon);
	Image_Rank->SetVisibility((nullptr != InPassiveInfo.RankIcon) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}
