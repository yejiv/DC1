// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/DC1HUDPalIcon.h"

#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UDC1HUDPalIcon::NativeConstruct()
{
	Super::NativeConstruct();

	Image_Icon->SetVisibility(ESlateVisibility::Collapsed);
	Image_Outline->SetVisibility(ESlateVisibility::Collapsed);
}

void UDC1HUDPalIcon::SetIcon(UTexture2D* InIcon)
{
	Image_Icon->SetVisibility(nullptr == InIcon ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	Image_Icon->SetBrushFromTexture(InIcon);
}

void UDC1HUDPalIcon::SetDead(bool bInDead)
{
	if (bInDead)
	{
		Image_Background->SetColorAndOpacity(Color_Red);
	}
	else
	{
		Image_Background->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.f));
	}
}

void UDC1HUDPalIcon::SetDeployed(bool bInDeployed)
{
	Image_Outline->SetVisibility(bInDeployed ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}