// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Bar/DC1Slider.h"
#include "Materials/MaterialInstanceDynamic.h"

void UDC1Slider::SynchronizeProperties()
{
	ApplyBarMaterial();

	// WidgetStyle을 SSlider로 넘김 -> WidgetStyle에 MID를 그 전에 세팅 ( ApplyBarMaterial )
	Super::SynchronizeProperties();

	RefreshFill();

	// 런타임에만 바인딩
	if (false == IsDesignTime())
	{
		OnValueChanged.AddUniqueDynamic(this, &ThisClass::HandleFillValueChanged);
	}
}

void UDC1Slider::ApplyBarMaterial()
{
	if (nullptr == BarMaterial)
	{
		return;
	}

	// MID 처음에 생성
	if (nullptr == BarMID || BarMID->Parent != BarMaterial)
	{
		BarMID = UMaterialInstanceDynamic::Create(BarMaterial, this);
	}

	FSliderStyle Style = GetWidgetStyle();

	Style.NormalBarImage.SetResourceObject(BarMID);
	Style.NormalBarImage.DrawAs = ESlateBrushDrawType::Image;

	Style.HoveredBarImage.SetResourceObject(BarMID);
	Style.HoveredBarImage.DrawAs = ESlateBrushDrawType::Image;

	Style.DisabledBarImage.SetResourceObject(BarMID);
	Style.DisabledBarImage.DrawAs = ESlateBrushDrawType::Image;

	SetWidgetStyle(Style);
}

void UDC1Slider::RefreshFill()
{
	if (nullptr == BarMID)
	{
		return;
	}
	// GetNormalizedValue = 0~1 정규화된 값으로 줌
	BarMID->SetScalarParameterValue(PercentParameterName, GetNormalizedValue());
}

void UDC1Slider::HandleFillValueChanged(float InValue)
{
	RefreshFill();
}
