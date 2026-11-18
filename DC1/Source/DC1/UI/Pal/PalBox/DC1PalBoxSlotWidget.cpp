// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Pal/PalBox/DC1PalBoxSlotWidget.h"

#include "UI/Bar/DC1StatusBar.h"

#include "Animation/WidgetAnimation.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDC1PalBoxSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Panel_Hover->SetVisibility(ESlateVisibility::Hidden);
}

void UDC1PalBoxSlotWidget::SetSlotInfo(const FDC1PalBoxSlotInfo& InSlotInfo)
{
	const FNumberFormattingOptions NoGrouping = FNumberFormattingOptions::DefaultNoGrouping();

	Panel_Content->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	Text_Level->SetText(FText::AsNumber(InSlotInfo.Level, &NoGrouping));
	Text_Name->SetText(InSlotInfo.Name);
	Text_Health->SetText(FText::AsNumber(FMath::RoundToInt(InSlotInfo.Health), &NoGrouping));
	Text_MaxHealth->SetText(FText::AsNumber(FMath::RoundToInt(InSlotInfo.MaxHealth), &NoGrouping));

	HealthBar->SnapToPercent((InSlotInfo.MaxHealth > 0.f) ? (InSlotInfo.Health / InSlotInfo.MaxHealth) : 0.f);

	Image_Icon->SetBrushFromTexture(InSlotInfo.Icon);
}

void UDC1PalBoxSlotWidget::SetEmpty()
{
	Panel_Content->SetVisibility(ESlateVisibility::Collapsed);
}

void UDC1PalBoxSlotWidget::HandleHovered()
{
	Super::HandleHovered();

	Panel_Hover->SetVisibility(ESlateVisibility::HitTestInvisible);

	FDC1AnimPlayInfo ForwardInfo;
	ForwardInfo.PlayMode = EUMGSequencePlayMode::Forward;

	PlayAnim(HoverInAnim, ForwardInfo);

	FDC1AnimPlayInfo LoopInfo;

	// 호버 동안 무한 루프, 언호버 시 원래 값으로 복원
	LoopInfo.PlayMode = EUMGSequencePlayMode::Forward;
	LoopInfo.RestoreState = true;
	LoopInfo.NumLoops = 0;

	PlayAnim(HoverLoopAnim, LoopInfo);

	OnSlotHovered.Broadcast(SlotIndex);
}

void UDC1PalBoxSlotWidget::HandleUnhovered()
{
	Super::HandleUnhovered();

	Panel_Hover->SetVisibility(ESlateVisibility::Hidden);

	FDC1AnimPlayInfo ReverseInfo;
	ReverseInfo.PlayMode = EUMGSequencePlayMode::Reverse;

	PlayAnim(HoverInAnim, ReverseInfo);

	StopAnim(HoverLoopAnim);
}
