// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1HUDBaseInfoRow.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

/**
 * 거점 정보 HUD 의 팰 한 줄. 팰 아이콘 + 일하는 중 망치 + 상태 문구
 */
UCLASS()
class DC1_API UDC1HUDBaseInfoRow : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void					SetRow(UTexture2D* InPalIcon, const FText& InConditionText, bool InWorking);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>		Image_PalIcon = { nullptr };

	// 일하는 중에만 보인다
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>		Image_Work = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>	Text_Condition = { nullptr };
};
