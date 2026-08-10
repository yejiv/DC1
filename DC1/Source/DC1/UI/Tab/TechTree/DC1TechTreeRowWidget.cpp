// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/TechTree/DC1TechTreeRowWidget.h"

#include "Components/TextBlock.h"

void UDC1TechTreeRowWidget::InitTier(int32 Level)
{
	if (false == ::IsValid(LevelText))
	{
		return;
	}

	LevelText->SetText(FText::AsNumber(Level));
}