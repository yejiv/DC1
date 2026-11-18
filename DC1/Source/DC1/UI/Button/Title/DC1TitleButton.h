// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/DC1UserButton.h"
#include "DC1TitleButton.generated.h"


class UButton;
class UTextBlock;
class UImage;
class UMaterialInstanceDynamic;

UCLASS()
class DC1_API UDC1TitleButton : public UDC1UserButton
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	virtual void HandleHovered() override;
	virtual void HandleUnhovered() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>			TextLabel = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				HoverImage = { nullptr };

	UPROPERTY(EditAnywhere, Category = "Button_LabelText")
	FText							Button_LabelText;

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> HoverMID = { nullptr };

	UPROPERTY(EditAnywhere, Category = "Hover")
	float	HoverSpeed = { 8.f };

	UPROPERTY(EditAnywhere, Category = "Hover")
	FLinearColor	HoverColor = { FLinearColor::White };

private:
	float			HoverProgress = { 0.f };
	bool			IsHovered = { false };

	FVector2D		NormalScale = { FVector2D(1.f) };
	FVector2D		HoverScale = { FVector2D(1.2f) };

	FLinearColor	NormalColor = { FLinearColor::White };
};
