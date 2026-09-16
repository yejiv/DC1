// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Bar/DC1StatusBar.h"

#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

void UDC1StatusBar::SetPercent(float InPercent)
{
	const float NewPercent = FMath::Clamp(InPercent, 0.f, 1.f);
	if (FMath::IsNearlyEqual(Percent, NewPercent))
	{
		return;
	}

	const bool IsDecreased = NewPercent < Percent;
	Percent = NewPercent;

	// 지연 감소를 안 쓰거나 값이 늘었으면 뒤바를 즉시 맞춘다
	if (false == UseDelayedBar || false == IsDecreased)
	{
		DelayedPercent = Percent;
		RemainingDelay = 0.f;
	}
	else
	{
		RemainingDelay = CatchUpDelay;
	}

	ApplyParameters();
}

void UDC1StatusBar::SnapToPercent(float InPercent)
{
	Percent = FMath::Clamp(InPercent, 0.f, 1.f);
	DelayedPercent = Percent;
	RemainingDelay = 0.f;

	ApplyParameters();
}

void UDC1StatusBar::NativePreConstruct()
{
	Super::NativePreConstruct();

	DelayedPercent = Percent;
	RemainingDelay = 0.f;

	ApplyParameters();
}

void UDC1StatusBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (false == IsCatchingUp())
	{
		return;
	}

	if (RemainingDelay > 0.f)
	{
		RemainingDelay -= InDeltaTime;
		return;
	}

	DelayedPercent = FMath::FInterpConstantTo(DelayedPercent, Percent, InDeltaTime, CatchUpSpeed);
	if (FMath::IsNearlyEqual(DelayedPercent, Percent))
	{
		DelayedPercent = Percent;
	}

	ApplyParameters();
}

void UDC1StatusBar::SetBarMaterial(UMaterialInterface* InMaterial)
{
	if (false == ::IsValid(BarImage) || false == ::IsValid(InMaterial))
	{
		return;
	}

	BarImage->SetBrushFromMaterial(InMaterial);
	ApplyParameters();
}

void UDC1StatusBar::ApplyParameters()
{
	if (false == ::IsValid(BarImage))
	{
		return;
	}

	// Brush에 머티리얼이 아니라 텍스처가 물려 있으면 null이 온다
	UMaterialInstanceDynamic* DynamicMaterial = BarImage->GetDynamicMaterial();

	if (nullptr == DynamicMaterial)
	{
		return;
	}

	DynamicMaterial->SetScalarParameterValue(PercentParameterName, Percent);

	if (true == UseDelayedBar)
	{
		DynamicMaterial->SetScalarParameterValue(DelayedPercentParameterName, DelayedPercent);
	}
}

bool UDC1StatusBar::IsCatchingUp() const
{
	if (false == UseDelayedBar)
	{
		return false;
	}

	return false == FMath::IsNearlyEqual(DelayedPercent, Percent);
}