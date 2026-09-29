// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Pal/PalBox/DC1PalBoxSkillRowWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDC1PalBoxSkillRowWidget::SetSkillInfo(const FDC1PalBoxSkillInfo& InSkillInfo)
{
	Panel_Skill->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	Panel_Empty->SetVisibility(ESlateVisibility::Collapsed);

	Text_Name->SetText(InSkillInfo.Name);
	Image_Element->SetBrushFromTexture(InSkillInfo.ElementIcon);
	Text_Power->SetText(FText::AsNumber(InSkillInfo.Power));
}

void UDC1PalBoxSkillRowWidget::SetEmpty()
{
	Panel_Skill->SetVisibility(ESlateVisibility::Collapsed);
	Panel_Empty->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}
