// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Notification/DC1FeedNoticeRowWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "TimerManager.h"

void UDC1FeedNoticeRowWidget::NativeDestruct()
{
	StopNotice();

	Super::NativeDestruct();
}

void UDC1FeedNoticeRowWidget::ShowNotice(const FDC1FeedNoticeInfo& InInfo)
{
	Image_Icon->SetBrushFromTexture(InInfo.Icon);
	Text_Message->SetText(InInfo.Message);

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

void UDC1FeedNoticeRowWidget::StopNotice()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimer);
	}

	StopAllAnimations();
}

void UDC1FeedNoticeRowWidget::BeginHide()
{
	if (nullptr == OutroAnim)
	{
		OnRowFinished.ExecuteIfBound(this);
		return;
	}

	PlayAnim(OutroAnim, FDC1AnimPlayInfo());
}

void UDC1FeedNoticeRowWidget::OnAnimFinished(const UWidgetAnimation* InAnim)
{
	if (InAnim == OutroAnim)
	{
		OnRowFinished.ExecuteIfBound(this);
	}
}
