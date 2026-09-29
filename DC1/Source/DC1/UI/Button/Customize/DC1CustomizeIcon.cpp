// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Button/Customize/DC1CustomizeIcon.h"

#include "Components/Image.h"

void UDC1CustomizeIcon::Init(const FGameplayTag& InPartTag, UTexture2D* InIcon)
{
	PartTag = InPartTag;
	IconImage->SetBrushFromTexture(InIcon);

	SelectedImage->SetOpacity(0.f);
}

void UDC1CustomizeIcon::SetSelected(bool bInSelected)
{
	SelectedImage->SetOpacity(bInSelected ? 1.f : 0.f);
}
