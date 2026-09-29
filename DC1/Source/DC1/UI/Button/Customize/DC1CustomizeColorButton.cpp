// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Button/Customize/DC1CustomizeColorButton.h"

#include "Components/Image.h"

void UDC1CustomizeColorButton::SetPreviewColor(const FLinearColor& InColor)
{
	if (true == IsValid(PreviewImage))
	{
		PreviewImage->SetColorAndOpacity(InColor);
	}
}
