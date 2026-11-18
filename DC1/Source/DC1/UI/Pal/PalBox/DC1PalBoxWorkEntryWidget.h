// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "ViewModel/Pal/DC1PalBoxDisplayTypes.h"
#include "DC1PalBoxWorkEntryWidget.generated.h"

class UImage;
class UTextBlock;

/**
 * 작업 적성 아이콘 하나
 */
UCLASS()
class DC1_API UDC1PalBoxWorkEntryWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void			SetWorkInfo(const FDC1PalBoxWorkInfo& InWorkInfo);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>			Image_Icon = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Level = { nullptr };

	// 적성 없음 투명도
	UPROPERTY(EditDefaultsOnly, Category = "PalBox", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float						DisabledOpacity = { 0.3f };
};
