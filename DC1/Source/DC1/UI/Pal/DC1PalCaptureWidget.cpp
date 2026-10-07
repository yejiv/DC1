// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Pal/DC1PalCaptureWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace DC1PalCapture
{
	// 링 머티리얼 파라미터
	static const FName PercentParam = FName("Percent");
	static const FName FillColorParam = FName("FillColor");
}

void UDC1PalCaptureWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 브러시에 텍스처가 물려 있으면 null
	RingMaterial = Image_Ring->GetDynamicMaterial();

	if (nullptr != RingMaterial)
	{
		RingMaterial->SetScalarParameterValue(DC1PalCapture::PercentParam, 0.f);
	}
}

void UDC1PalCaptureWidget::SetCaptureState(int32 InStage, float InChance, bool bInSuccess)
{
	const FLinearColor TargetColor = (true == bInSuccess)
		? SuccessColor
		: ((0 < StageColors.Num()) ? StageColors[FMath::Clamp(InStage, 0, StageColors.Num() - 1)] : SuccessColor);

	if (INDEX_NONE == CurrentStage)
	{
		CurrentFill = 0.f;
		CurrentColor = TargetColor;
	}

	CurrentStage = InStage;
	bSuccess = bInSuccess;

	FillFrom = CurrentFill;
	FillTo = FMath::Clamp(InChance, 0.f, 1.f);

	ColorFrom = CurrentColor;
	ColorTo = TargetColor;

	FillElapsed = 0.f;

	RefreshDisplay();
}

void UDC1PalCaptureWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if ((INDEX_NONE == CurrentStage) || (FillElapsed >= FillDuration))
	{
		return;
	}

	FillElapsed = FMath::Min(FillElapsed + InDeltaTime, FillDuration);

	// 끝에서 느려진다
	const float Alpha = FillElapsed / FillDuration;
	const float Eased = 1.f - FMath::Square(1.f - Alpha);

	CurrentFill = FMath::Lerp(FillFrom, FillTo, Eased);
	CurrentColor = FMath::Lerp(ColorFrom, ColorTo, Eased);

	RefreshDisplay();
}

void UDC1PalCaptureWidget::RefreshDisplay()
{
	int32 Percent = FMath::FloorToInt((CurrentFill * 100.f) + KINDA_SMALL_NUMBER);

	if ((false == bSuccess) && (FillTo < 1.f))
	{
		Percent = FMath::Min(Percent, 99);
	}

	Text_Chance->SetText(FText::AsNumber(Percent));

	if (nullptr != RingMaterial)
	{
		RingMaterial->SetScalarParameterValue(DC1PalCapture::PercentParam, CurrentFill);
		RingMaterial->SetVectorParameterValue(DC1PalCapture::FillColorParam, CurrentColor);
	}

	if (nullptr != Image_Accent)
	{
		Image_Accent->SetColorAndOpacity(CurrentColor);
	}
}
