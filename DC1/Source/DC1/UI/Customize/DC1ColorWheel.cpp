// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Customize/DC1ColorWheel.h"

#include "Components/Image.h"
#include "Components/Slider.h"
#include "Engine/Texture2D.h"

#include "UI/Button/DC1UserButton.h"

void UDC1ColorWheel::SetSeedColor(const FLinearColor& InColor)
{
	const FLinearColor HSVColor = InColor.LinearRGBToHSV();
	Hue = HSVColor.R;
	Saturation = HSVColor.G;
	Value = HSVColor.B;

	if (true == IsValid(WheelImage))
	{
		RepositionIndicator(WheelImage->GetCachedGeometry().GetLocalSize());
	}

	if (true == IsValid(ValueSlider))
	{
		ValueSlider->SetValue(Value);
	}
}

void UDC1ColorWheel::NativeConstruct()
{
	Super::NativeConstruct();

	GenerateWheelTexture();

	if (true == IsValid(ValueSlider))
	{
		ValueSlider->OnValueChanged.AddDynamic(this, &ThisClass::HandleValueChanged);
	}

	if (true == IsValid(ConfirmButton))
	{
		ConfirmButton->OnClickedDelegate.AddWeakLambda(this, [this]() { HandleConfirmClicked(); });
	}

	if (true == IsValid(CancelButton))
	{
		CancelButton->OnClickedDelegate.AddWeakLambda(this, [this]() { HandleCancelClicked(); });
	}
}

void UDC1ColorWheel::NativeDestruct()
{
	if (true == IsValid(ValueSlider))
	{
		ValueSlider->OnValueChanged.RemoveDynamic(this, &ThisClass::HandleValueChanged);
	}

	if (true == IsValid(ConfirmButton))
	{
		ConfirmButton->OnClickedDelegate.RemoveAll(this);
	}

	if (true == IsValid(CancelButton))
	{
		CancelButton->OnClickedDelegate.RemoveAll(this);
	}

	Super::NativeDestruct();
}

FReply UDC1ColorWheel::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	bIsDragging = true;

	const FGeometry WheelImageGeometry = WheelImage->GetCachedGeometry();

	const FVector2D LocalPosition = WheelImageGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	UpdateFromLocalPosition(LocalPosition, WheelImageGeometry.GetLocalSize());

	return FReply::Handled().CaptureMouse(TakeWidget());
}

FReply UDC1ColorWheel::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (false == bIsDragging)
	{
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}

	const FGeometry WheelImageGeometry = WheelImage->GetCachedGeometry();

	const FVector2D LocalPosition = WheelImageGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	UpdateFromLocalPosition(LocalPosition, WheelImageGeometry.GetLocalSize());

	return FReply::Handled();
}

FReply UDC1ColorWheel::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	}

	bIsDragging = false;

	return FReply::Handled().ReleaseMouseCapture();
}

void UDC1ColorWheel::GenerateWheelTexture()
{
	TArray<FColor> PixelData;
	PixelData.SetNumUninitialized(WheelResolution * WheelResolution);

	// 중심을 (0,0), 가장자리를 반지름 1로 두는 정규화 좌표
	const float Center = (WheelResolution - 1) * 0.5f;

	for (int32 Y = 0; Y < WheelResolution; ++Y)
	{
		for (int32 X = 0; X < WheelResolution; ++X)
		{
			const float NormX = (X - Center) / Center;
			const float NormY = (Y - Center) / Center;
			const float Radius = FMath::Sqrt(NormX * NormX + NormY * NormY);

			FColor PixelColor = FColor(0, 0, 0, 0);
			if (Radius <= 1.f)
			{
				float AngleDegrees = FMath::RadiansToDegrees(FMath::Atan2(NormY, NormX));
				if (AngleDegrees < 0.f)
				{
					AngleDegrees += 360.f;
				}

				const FLinearColor HSVColor(AngleDegrees, Radius, 1.f);
				PixelColor = HSVColor.HSVToLinearRGB().ToFColor(true);
			}

			PixelData[Y * WheelResolution + X] = PixelColor;
		}
	}

	WheelTexture = UTexture2D::CreateTransient(WheelResolution, WheelResolution, PF_B8G8R8A8, NAME_None,
		TConstArrayView64<uint8>(reinterpret_cast<const uint8*>(PixelData.GetData()), PixelData.Num() * sizeof(FColor)));
	if (false == IsValid(WheelTexture))
	{
		return;
	}

	if (true == IsValid(WheelImage))
	{
		WheelImage->SetBrushFromTexture(WheelTexture);
	}
}

void UDC1ColorWheel::UpdateFromLocalPosition(const FVector2D& InLocalPosition, const FVector2D& InLocalSize)
{
	const FVector2D Center = InLocalSize * 0.5f;
	const float HalfExtent = FMath::Min(Center.X, Center.Y);
	if (HalfExtent <= 0.f)
	{
		return;
	}

	const FVector2D Delta = InLocalPosition - Center;

	float AngleDegrees = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
	if (AngleDegrees < 0.f)
	{
		AngleDegrees += 360.f;
	}

	Hue = AngleDegrees;
	Saturation = FMath::Clamp(Delta.Size() / HalfExtent, 0.f, 1.f);

	RepositionIndicator(InLocalSize);

	BroadcastColorChanged();
}

void UDC1ColorWheel::HandleValueChanged(float InValue)
{
	Value = InValue;

	BroadcastColorChanged();
}

void UDC1ColorWheel::HandleConfirmClicked()
{
	OnPickClosed.Broadcast(true);
}

void UDC1ColorWheel::HandleCancelClicked()
{
	OnPickClosed.Broadcast(false);
}

void UDC1ColorWheel::RepositionIndicator(const FVector2D& InLocalSize)
{
	if (false == IsValid(Indicator))
	{
		return;
	}

	const FVector2D Center = InLocalSize * 0.5f;
	const float HalfExtent = FMath::Min(Center.X, Center.Y);
	const float AngleRadians = FMath::DegreesToRadians(Hue);

	const FVector2D Offset = FVector2D(FMath::Cos(AngleRadians), FMath::Sin(AngleRadians)) * Saturation * HalfExtent;

	Indicator->SetRenderTranslation(Offset);
}

void UDC1ColorWheel::BroadcastColorChanged()
{
	const FLinearColor HSVColor(Hue, Saturation, Value);
	OnColorChanged.Broadcast(HSVColor.HSVToLinearRGB());
}
