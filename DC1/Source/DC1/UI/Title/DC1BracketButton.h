// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1BracketButton.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UWidgetAnimation;

UCLASS()
class DC1_API UDC1BracketButton : public UDC1UserWidget
{
	GENERATED_BODY()
	
public:
	FSimpleMulticastDelegate OnClickedDelegate;
	
protected:
	virtual void	NativeConstruct() override;

	UFUNCTION() 
	void			HandleClicked();
	UFUNCTION() 
	void			HandleHovered();
	UFUNCTION() 
	void			HandleUnhovered();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton>				BracketButton;
	
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation>	HoverAnim;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>			TextLabel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				BracketImage;

	UPROPERTY(EditAnywhere, Category = "Button_LabelText")
	FText							Button_LabelText;
};
