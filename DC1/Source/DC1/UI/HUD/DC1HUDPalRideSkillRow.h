// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1HUDPalRideSkillRow.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

/**
 * 라이딩 HUD 의 탑승 스킬 한 줄. 키 이미지 + 스킬 이름
 */
UCLASS()
class DC1_API UDC1HUDPalRideSkillRow : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void					SetSkill(const FText& InName, UTexture2D* InKeyIcon);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>		Image_Key = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>	Text_Name = { nullptr };
};
