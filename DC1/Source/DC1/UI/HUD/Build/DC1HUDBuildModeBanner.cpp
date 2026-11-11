// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/Build/DC1HUDBuildModeBanner.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"

void UDC1HUDBuildModeBanner::SetMode(EDC1BuildMode InMode)
{
	if (true == ::IsValid(Text_Mode))
	{
		Text_Mode->SetText(ModeTexts.FindRef(InMode));
	}

	if (true == ::IsValid(ModeImageBox))
	{
		const FLinearColor* ModeColor = ModeColors.Find(InMode);
		ModeImageBox->SetContentColorAndOpacity((nullptr != ModeColor) ? *ModeColor : FLinearColor::White);
	}
}
