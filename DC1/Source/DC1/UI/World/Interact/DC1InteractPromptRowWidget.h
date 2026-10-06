// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1InteractPromptRowWidget.generated.h"

struct FDC1InteractOption;

class UTextBlock;
class UProgressBar;

UCLASS()
class DC1_API UDC1InteractPromptRowWidget : public UDC1UserWidget
{
	GENERATED_BODY()
	
public:
	void	SetOption(const FDC1InteractOption& InOption);
	void	SetFillRatio(float InRatio);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		KeyText = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		DisplayText = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar>	FillBar = { nullptr };
};
