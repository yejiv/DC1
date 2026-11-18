// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/DC1UserButton.h"
#include "DC1CustomizeColorButton.generated.h"

class UImage;

/**
 * 현재 선택된 피부색/머리색을 스와치로 미리 보여주는 버튼. 클릭되면 Panel이 ColorWheel 팝업을 연다.
 */
UCLASS()
class DC1_API UDC1CustomizeColorButton : public UDC1UserButton
{
	GENERATED_BODY()

public:
	void SetPreviewColor(const FLinearColor& InColor);

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage>		PreviewImage = { nullptr };
};
