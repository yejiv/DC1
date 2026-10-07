// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Interface/DC1InteractableInterface.h"
#include "DC1InteractPromptRowWidget.generated.h"

class UTextBlock;
class UImage;
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
	TObjectPtr<UImage>			KeyImage = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		DisplayText = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar>	FillBar = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	TMap<EDC1InteractKey, TObjectPtr<UTexture2D>> KeyIcons;
};
