// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1TitleButton.generated.h"


class UButton;
class UTextBlock;
class UImage;
class UMaterialInstanceDynamic;

UCLASS()
class DC1_API UDC1TitleButton : public UDC1UserWidget
{
	GENERATED_BODY()
	
public:
	FSimpleMulticastDelegate OnClickedDelegate;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void HandleClicked();

	UFUNCTION()
	void HandleHovered();

	UFUNCTION()
	void HandleUnhovered();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton>				TitleButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>			TextLabel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				HoverImage;

	UPROPERTY(EditAnywhere, Category = "Button_LabelText")
	FText							Button_LabelText;

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> HoverMID;

	float	HoverProgress = 0.f;
	bool	IsHovered = false;

	FVector2D	NormalScale = FVector2D(1.f);
	FVector2D	HoverScale = FVector2D(1.2f);

	FLinearColor	NormalColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Hover")
	float	HoverSpeed = 8.f;

	UPROPERTY(EditAnywhere, Category = "Hover")
	FLinearColor	HoverColor = FLinearColor::White;
};
