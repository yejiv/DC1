// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "DC1ScreenOffsetWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class DC1_API UDC1ScreenOffsetWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	void		SetScreenOffset(const FVector2D& InScreenOffset) { ScreenOffset = InScreenOffset; }

protected:
	virtual FVector2D ModifyProjectedLocalPosition(const FGeometry& ViewportGeometry, const FVector2D& LocalPosition) override;

private:
	FVector2D	ScreenOffset = { FVector2D::ZeroVector };
};
