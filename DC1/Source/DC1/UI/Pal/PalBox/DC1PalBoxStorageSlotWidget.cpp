// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Pal/PalBox/DC1PalBoxStorageSlotWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Components/Image.h"

void UDC1PalBoxStorageSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (nullptr != Panel_Hover)
	{
		Panel_Hover->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UDC1PalBoxStorageSlotWidget::SetSlotInfo(const FDC1PalBoxSlotInfo& InSlotInfo)
{
	Panel_Content->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	Image_Icon->SetBrushFromTexture(InSlotInfo.Icon);
}

void UDC1PalBoxStorageSlotWidget::SetEmpty()
{
	Panel_Content->SetVisibility(ESlateVisibility::Collapsed);
}

void UDC1PalBoxStorageSlotWidget::HandleHovered()
{
	Super::HandleHovered();

	if (nullptr != Panel_Hover)
	{
		Panel_Hover->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	FDC1AnimPlayInfo LoopInfo;

	// 호버 동안 무한 루프, 언호버 시 원래 값으로 복원
	LoopInfo.PlayMode = EUMGSequencePlayMode::Forward;
	LoopInfo.bRestoreState = true;
	LoopInfo.NumLoops = 0;

	PlayAnim(HoverLoopAnim, LoopInfo);

	OnSlotHovered.Broadcast(SlotIndex);
}

void UDC1PalBoxStorageSlotWidget::HandleUnhovered()
{
	Super::HandleUnhovered();

	if (nullptr != Panel_Hover)
	{
		Panel_Hover->SetVisibility(ESlateVisibility::Hidden);
	}

	StopAnim(HoverLoopAnim);
}
