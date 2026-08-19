// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DC1UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Animation/WidgetAnimationState.h"


void UDC1UserWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 위젯 단위로 한 번만 바인딩
	OnAnimationFinishedPlaying().AddUObject(this, &ThisClass::HandleAnimFinished);
}

void UDC1UserWidget::NativeDestruct()
{
	for (const TPair<TObjectPtr<UWidgetAnimation>, FDC1AnimPlayInfo>& Pair : PlayingAnims)
	{
		SnapAnimToEdge(Pair.Key, Pair.Value);
	}

	PlayingAnims.Empty();

	Super::NativeDestruct();
}

void UDC1UserWidget::HandleInputKey(const FKey& InKey)
{
}

void UDC1UserWidget::Activate()
{
	FDC1AnimPlayInfo PlayInfo;
	PlayInfo.PlayMode = EUMGSequencePlayMode::Forward;

	PlayAnim(IntroAnim, PlayInfo);

	NativeOnActivated();
}

void UDC1UserWidget::Deactivate()
{
	FDC1AnimPlayInfo PlayInfo;
	PlayInfo.PlayMode = EUMGSequencePlayMode::Forward;

	PlayAnim(OutroAnim, PlayInfo);

	NativeOnDeactivated();
}

void UDC1UserWidget::NativeOnActivated()
{
}

void UDC1UserWidget::NativeOnDeactivated()
{
}

void UDC1UserWidget::PlayAnim(UWidgetAnimation* InAnim, const FDC1AnimPlayInfo& InPlayInfo)
{
	if (false == ::IsValid(InAnim))
	{
		return;
	}

	if (true == IsAnimationPlaying(InAnim))
	{
		const FDC1AnimPlayInfo* CurrentInfo = PlayingAnims.Find(InAnim);

		if (nullptr != CurrentInfo && (CurrentInfo->PlayMode) == (InPlayInfo.PlayMode))
		{
			return;
		}

		ReverseAnimation(InAnim);
	}
	else
	{
		PlayAnimation(InAnim, 0.f, InPlayInfo.NumLoops, InPlayInfo.PlayMode, 1.f, InPlayInfo.bRestoreState);
	}

	PlayingAnims.Add(InAnim, InPlayInfo);
}

void UDC1UserWidget::SnapAnimToEdge(UWidgetAnimation* InAnim, const FDC1AnimPlayInfo& InPlayInfo)
{
	if (false == ::IsValid(InAnim))
	{
		return;
	}

	StopAnimation(InAnim);

	if (true == InPlayInfo.bRestoreState)
	{
		return;
	}

	const float SnapTime = (InPlayInfo.PlayMode == EUMGSequencePlayMode::Forward) ? InAnim->GetEndTime() : InAnim->GetStartTime();

	PlayAnimationTimeRange(InAnim, SnapTime, SnapTime);
}

void UDC1UserWidget::StopAnim(UWidgetAnimation* InAnim)
{
	if (false == ::IsValid(InAnim))
	{
		return;
	}

	StopAnimation(InAnim);

	PlayingAnims.Remove(InAnim);
}

void UDC1UserWidget::HandleAnimFinished(FWidgetAnimationState& InState)
{
	const UWidgetAnimation* FinishedAnim = InState.GetAnimation();
	if (nullptr == FinishedAnim)
	{
		return;
	}

	PlayingAnims.Remove(const_cast<UWidgetAnimation*>(FinishedAnim));

	OnAnimFinished(FinishedAnim);
}
