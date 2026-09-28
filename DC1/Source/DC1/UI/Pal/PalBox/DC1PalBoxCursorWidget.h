// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1PalBoxCursorWidget.generated.h"

class UImage;
class UTexture2D;

/**
 * 집은 팰. 마우스를 따라다닌다
 */
UCLASS()
class DC1_API UDC1PalBoxCursorWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void			SetIcon(UTexture2D* InIcon);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>			Image_Icon = { nullptr };
};
