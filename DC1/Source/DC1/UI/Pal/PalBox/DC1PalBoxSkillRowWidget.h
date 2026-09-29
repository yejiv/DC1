// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "ViewModel/Pal/DC1PalBoxDisplayTypes.h"
#include "DC1PalBoxSkillRowWidget.generated.h"

class UImage;
class UTextBlock;

/**
 * 액티브 스킬 한 줄
 */
UCLASS()
class DC1_API UDC1PalBoxSkillRowWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void			SetSkillInfo(const FDC1PalBoxSkillInfo& InSkillInfo);

	// "스킬 없음"
	void			SetEmpty();

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidget>			Panel_Skill = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidget>			Panel_Empty = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Name = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>			Image_Element = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Power = { nullptr };
};
