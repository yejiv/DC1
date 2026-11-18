// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Define/DC1Enums.h"
#include "DC1HUDBuildModeBanner.generated.h"

class UBorder;
class UTextBlock;

UCLASS()
class DC1_API UDC1HUDBuildModeBanner : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void SetMode(EDC1BuildMode InMode);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UBorder>				ModeImageBox = { nullptr };

	// 현재 모드 Text
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>			Text_Mode = { nullptr };

private:
	// 모드별 텍스트 Build - 건설모드, Dismantle - 해체모드
	UPROPERTY(EditAnywhere, Category = "HUD|Build")
	TMap<EDC1BuildMode, FText>			ModeTexts;

	UPROPERTY(EditAnywhere, Category = "HUD|Build|Color")
	TMap<EDC1BuildMode, FLinearColor>	ModeColors;
};
