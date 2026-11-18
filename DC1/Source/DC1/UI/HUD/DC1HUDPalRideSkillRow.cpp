// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/DC1HUDPalRideSkillRow.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UDC1HUDPalRideSkillRow::SetSkill(const FText& InName, UTexture2D* InKeyIcon)
{
	Image_Key->SetBrushFromTexture(InKeyIcon);
	Text_Name->SetText(InName);

	FlashRemaining = 0.f;
	Image_Flash->SetRenderOpacity(0.f);

	SetCoolingDown(false);
}

void UDC1HUDPalRideSkillRow::SetCoolingDown(bool InCoolingDown)
{
	// 이름과 막대는 같은 자리. 줄 폭이 변하지 않게 이름은 Hidden
	Text_Name->SetVisibility(InCoolingDown ? ESlateVisibility::Hidden : ESlateVisibility::SelfHitTestInvisible);
	ProgressBar_Cooldown->SetVisibility(InCoolingDown ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	Image_Key->SetRenderOpacity(InCoolingDown ? CoolingKeyOpacity : 1.f);

	if (true == InCoolingDown)
	{
		ProgressBar_Cooldown->SetPercent(0.f);
	}
}

void UDC1HUDPalRideSkillRow::SetCooldownRatio(float InRatio)
{
	ProgressBar_Cooldown->SetPercent(InRatio);
}

void UDC1HUDPalRideSkillRow::PlayCooldownFlash()
{
	FlashRemaining = FlashDuration;
	Image_Flash->SetRenderOpacity(1.f);
}

void UDC1HUDPalRideSkillRow::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (FlashRemaining <= 0.f)
		return;

	FlashRemaining = FMath::Max(FlashRemaining - InDeltaTime, 0.f);
	Image_Flash->SetRenderOpacity(FlashRemaining / FlashDuration);
}
