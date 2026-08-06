// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Fade/DC1FadeWidget.h"
#include "UI/Fade/DC1FadeImage.h"
#include "UI/Fade/DC1FadeText.h"

void UDC1FadeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetRenderOpacity(0.f);
	SetVisibility(ESlateVisibility::Collapsed);
}

void UDC1FadeWidget::NativeTick(const FGeometry& MyGeomtry, float InDeltaTime)
{
	Super::NativeTick(MyGeomtry, InDeltaTime);

	if (false == bIsFading)
	{
		return;
	}

	Elapsed += InDeltaTime;

	const float Ratio = FMath::Clamp(Elapsed / FadeDuration, 0.f, 1.f);
	const float EasedRatio = FMath::InterpEaseIn(0.f, 1.f, Ratio, 2.f);

	SetRenderOpacity(FMath::Lerp(StartAlpha, TargetAlpha, EasedRatio));

	if (1.f <= Ratio)
	{
		FinishFade();
	}
}

void UDC1FadeWidget::SetFadeType(EDC1FadeType InType, const FUILoadingEntry* InLoadingEntry, const FText* InText)
{
	if (EDC1FadeType::Default == InType)
	{
		FadeImage->Stop();
		FadeImage->SetVisibility(ESlateVisibility::Collapsed);
		FadeText->SetVisibility(ESlateVisibility::Collapsed);

		return;
	}

	if (nullptr != InLoadingEntry)
	{
		FadeImage->Play(*InLoadingEntry);
		FadeImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	if (EDC1FadeType::Spinner == InType)
	{
		FadeText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (EDC1FadeType::Loading == InType && nullptr != InText)
	{
		FadeText->SetText(*InText);
		FadeText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UDC1FadeWidget::FadeIn(float InDuration, FSimpleDelegate OnComplete)
{
	if (true == bIsFading)
	{
		return;
	}

	StartFade(1.f, InDuration, OnComplete);
}

void UDC1FadeWidget::FadeOut(float InDuration, FSimpleDelegate OnComplete)
{
	if (true == bIsFading)
	{
		return;
	}

	StartFade(0.f, InDuration, OnComplete);
}

void UDC1FadeWidget::StartFade(float InTargetAlpha, float InDuration, FSimpleDelegate OnComplete)
{
	SetVisibility(ESlateVisibility::Visible);

	StartAlpha = GetRenderOpacity();
	TargetAlpha = InTargetAlpha;
	FadeDuration = InDuration;
	Elapsed = 0.f;
	CompleteDelegate = OnComplete;

	if (0.f >= InDuration)
	{
		SetRenderOpacity(TargetAlpha);
		FinishFade();

		return;
	}

	bIsFading = true;
}

void UDC1FadeWidget::FinishFade()
{
	bIsFading = false;

	if (0.f >= TargetAlpha)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		SetFadeType(EDC1FadeType::Default);
	}

	FSimpleDelegate Local = CompleteDelegate;
	CompleteDelegate.Unbind();
	Local.ExecuteIfBound();
}