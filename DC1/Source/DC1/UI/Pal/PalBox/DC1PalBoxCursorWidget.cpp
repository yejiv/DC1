// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Pal/PalBox/DC1PalBoxCursorWidget.h"

#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UDC1PalBoxCursorWidget::SetIcon(UTexture2D* InIcon)
{
	Image_Icon->SetBrushFromTexture(InIcon);
}
