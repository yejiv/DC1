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
	virtual void	NativeOnInitialized() override;
	virtual void	NativeConstruct() override;

protected:
	virtual void	HandleHovered() override;
	virtual void	HandleUnhovered() override;

protected:
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation>	HoverAnim = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox>			ButtonSizeBox = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				BracketImage = { nullptr };

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock>			TextLabel = { nullptr };

	UPROPERTY(EditAnywhere, Category = "UserButton")
	FText							Button_LabelText = { FText::GetEmpty()};

	UPROPERTY(EditAnywhere, Category = "UserButton")
	float	ButtonSizeX = { 0.f };

	UPROPERTY(EditAnywhere, Category = "UserButton")
	float	ButtonSizeY = { 0.f };

	UPROPERTY(EditAnywhere, Category = "UserButton")
	float	FontSize = { 0.f };

	UPROPERTY(EditAnywhere, Category = "UserButton|Image")
	bool	ButtonBrushOverride = { false };

	UPROPERTY(EditAnywhere, Category = "UserButton|Image", meta = (EditCondition = "ButtonBrushOverride"))
	FSlateBrush		NormalBrush;

	UPROPERTY(EditAnywhere, Category = "UserButton|Image", meta = (EditCondition = "ButtonBrushOverride"))
	FSlateBrush		HoveredBrush;

	UPROPERTY(EditAnywhere, Category = "UserButton|Image", meta = (EditCondition = "ButtonBrushOverride"))
	FSlateBrush		PressedBrush;
};
