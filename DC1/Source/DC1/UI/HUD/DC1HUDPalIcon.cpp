// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/DC1HUDPalIcon.h"

#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"

void UDC1HUDPalIcon::NativeConstruct()
{
	Super::NativeConstruct();

	// 브러시에 M_PalIcon이 물려 있어야 MID가 나온다. 텍스처가 물려 있으면 null
	IconMID = Image_Icon->GetDynamicMaterial();

	SetIcon(nullptr);
	SetDead(false);
	SetDeployed(false);
}

void UDC1HUDPalIcon::SetIcon(UTexture2D* InIcon)
{
	if (nullptr == IconMID)
		return;

	IconMID->SetTextureParameterValue(IconParameterName, InIcon);

	// 빈 칸이면 초상화를 완전히 끈다. 배경만 남는다
	IconMID->SetScalarParameterValue(IconOpacityParameterName, (nullptr != InIcon) ? 1.f : 0.f);
}

void UDC1HUDPalIcon::SetDead(bool bInDead)
{
	if (nullptr == IconMID)
		return;

	IconMID->SetVectorParameterValue(BackgroundColorParameterName, bInDead ? Color_Dead : Color_Normal);
}

void UDC1HUDPalIcon::SetDeployed(bool bInDeployed)
{
	Image_Outline->SetVisibility(bInDeployed ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}