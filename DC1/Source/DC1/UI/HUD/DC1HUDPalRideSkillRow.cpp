// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/DC1HUDPalRideSkillRow.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDC1HUDPalRideSkillRow::SetSkill(const FText& InName, UTexture2D* InKeyIcon)
{
	Image_Key->SetBrushFromTexture(InKeyIcon);
	Text_Name->SetText(InName);
}
