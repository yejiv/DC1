// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "ViewModel/Pal/DC1PalBoxDisplayTypes.h"
#include "DC1PalBoxPassiveRowWidget.generated.h"

class UImage;
class UTextBlock;

/**
 * 패시브 한 칸
 */
UCLASS()
class DC1_API UDC1PalBoxPassiveRowWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void			SetPassiveInfo(const FDC1PalBoxPassiveInfo& InPassiveInfo);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Name = { nullptr };

	// Rank 텍스처 없으면 숨김
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage>			Image_Rank = { nullptr };
};
