// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/DC1UserButton.h"
#include "DC1BracketButton.generated.h"

class UButton;
class UTextBlock;
class UImage;
class USizeBox;
class UWidgetAnimation;

UCLASS()
class DC1_API UDC1BracketButton : public UDC1UserButton
{
	GENERATED_BODY()
	
protected:
	virtual void	NativeConstruct() override;

protected:
	virtual void	HandleHovered() override;
	virtual void	HandleUnhovered() override;

protected:
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation>	HoverAnim;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox>			ButtonSizeBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				BracketImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>			TextLabel;

	UPROPERTY(EditAnywhere, Category = "UserButton")
	FText							Button_LabelText;

	UPROPERTY(EditAnywhere, Category = "UserButton")
	float	ButtonSizeX= 0.f;

	UPROPERTY(EditAnywhere, Category = "UserButton")
	float	ButtonSizeY = 0.f;

	UPROPERTY(EditAnywhere, Category = "UserButton")
	float	FontSize = 0.f;

	UPROPERTY(EditAnywhere, Category = "UserButton|Image")
	bool	ButtonBrushOverride = false;

	UPROPERTY(EditAnywhere, Category = "UserButton|Image", meta = (EditCondition = "ButtonBrushOverride"))
	FSlateBrush		NormalBrush;

	UPROPERTY(EditAnywhere, Category = "UserButton|Image", meta = (EditCondition = "ButtonBrushOverride"))
	FSlateBrush		HoveredBrush;

	UPROPERTY(EditAnywhere, Category = "UserButton|Image", meta = (EditCondition = "ButtonBrushOverride"))
	FSlateBrush		PressedBrush;
};
