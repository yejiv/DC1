// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/DC1HUDCompassWidget.h"

#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UDC1HUDCompassWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreateCompassMID();

	if(IsValid(CompassMID))
		CompassMID->SetScalarParameterValue(SpanParamName, SpanDegrees / 360.f);
}

void UDC1HUDCompassWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDC1HUDCompassWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (APlayerController* PC = GetOwningPlayer())
	{
		UVOffset = CalcUVOffset(PC->GetControlRotation().Yaw);
		ApplyToMaterial();
	}
}

void UDC1HUDCompassWidget::CreateCompassMID()
{
	if (false == IsValid(Image_Compass))
		return;

	CompassMID = Image_Compass->GetDynamicMaterial();
}

float UDC1HUDCompassWidget::CalcUVOffset(float InViewYaw) const
{
	return InViewYaw / 360.f - SpanDegrees / 360.f * 0.5f;
}

void UDC1HUDCompassWidget::ApplyToMaterial()
{
	if (false == IsValid(CompassMID))
		return;

	CompassMID->SetScalarParameterValue(OffsetParamName, UVOffset);
}
