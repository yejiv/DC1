// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UI/DC1UserWidget.h"
#include "DC1FadeText.generated.h"

class UTextBlock;

UCLASS()
class DC1_API UDC1FadeText : public UDC1UserWidget
{
	GENERATED_BODY()
	
public:
	void	SetText(const FText& InText);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		FadeTip;
};
