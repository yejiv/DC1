// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Pal/DC1PalCaptureNoticeWidget.h"

#include "UI/Pal/PalBox/DC1PalBoxPassiveRowWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Components/DynamicEntryBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "TimerManager.h"

void UDC1PalCaptureNoticeWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 알림이 올 때까지 숨김
	SetVisibility(ESlateVisibility::Collapsed);
}

void UDC1PalCaptureNoticeWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimer);
	}

	Super::NativeDestruct();
}

void UDC1PalCaptureNoticeWidget::ShowNotice(const FDC1PalCaptureNoticeInfo& InInfo)
{
	Image_Icon->SetBrushFromTexture(InInfo.Icon);
	Text_Name->SetText(InInfo.Name);

	// 04 처럼 두 자리
	FNumberFormattingOptions LevelFormat;
	LevelFormat.MinimumIntegralDigits = 2;
	Text_Level->SetText(FText::AsNumber(InInfo.Level, &LevelFormat));

	PassiveContainer->Reset();
	for (const FDC1PalBoxPassiveInfo& Passive : InInfo.Passives)
	{
		if (UDC1PalBoxPassiveRowWidget* Row = PassiveContainer->CreateEntry<UDC1PalBoxPassiveRowWidget>())
		{
			Row->SetPassiveInfo(Passive);
		}
	}

	Panel_Passive->SetVisibility(InInfo.Passives.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);

	// 입력을 받지 않는다
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (nullptr != IntroAnim)
	{
		PlayAnim(IntroAnim, FDC1AnimPlayInfo());
	}

	// 나타나는 시간 + 떠 있는 시간 뒤 사라지기 시작
	const float IntroLength = (nullptr != IntroAnim) ? IntroAnim->GetEndTime() : 0.f;
	GetWorld()->GetTimerManager().SetTimer(HideTimer, this, &ThisClass::BeginHide, IntroLength + DisplayDuration, false);
}

void UDC1PalCaptureNoticeWidget::BeginHide()
{
	if (nullptr == OutroAnim)
	{
		FinishHide();
		return;
	}

	PlayAnim(OutroAnim, FDC1AnimPlayInfo());
}

void UDC1PalCaptureNoticeWidget::OnAnimFinished(const UWidgetAnimation* InAnim)
{
	if (InAnim == OutroAnim)
	{
		FinishHide();
	}
}

void UDC1PalCaptureNoticeWidget::FinishHide()
{
	SetVisibility(ESlateVisibility::Collapsed);

	OnNoticeFinished.Broadcast();
}
