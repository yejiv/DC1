// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Widget/DC1ScreenOffsetWidgetComponent.h"

FVector2D UDC1ScreenOffsetWidgetComponent::ModifyProjectedLocalPosition(const FGeometry& ViewportGeometry, const FVector2D& LocalPosition)
{
	return Super::ModifyProjectedLocalPosition(ViewportGeometry, LocalPosition) + ScreenOffset;
}
