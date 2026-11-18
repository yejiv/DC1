// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/AnimButton/Tab/DC1TabHeaderButton.h"
#include "DC1BuildTabHeaderButton.generated.h"


UCLASS()
class DC1_API UDC1BuildTabHeaderButton : public UDC1TabHeaderButton
{
	GENERATED_BODY()

protected:
	virtual void	NativeOnInitialized() override;

protected:
	virtual void	ApplyState() override;

public:
	virtual bool	SetState(EDC1TabButtonState InState) override;

private:
	// Opacity 조절용 Overlay
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>	RootOverlay = { nullptr };

	// Inctive Anim Optional
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation>	InactivedAnim = { nullptr };

	// Active Hover Image
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>	ActiveHoverImage = { nullptr };

	// Button Icon Image
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>	ButtonIconImage = { nullptr };

	// Disable Opacity
	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Build|Image|Icon")
	TSoftObjectPtr<UTexture2D>	IconTexture = { nullptr };

	// State To Font Icon Color
	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Build|Image|Color")
	FLinearColor		ActiveFontIconColor = { FLinearColor::White };

	// State To Font Icon Color
	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Build|Image|Color")
	FLinearColor		InactiveFontIconColor = { FLinearColor::White };

	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Build|Image|Color")
	FLinearColor		ActiveHoverColor = { FLinearColor::White };

	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Build|Image|Color")
	FLinearColor		InactiveHoverColor = { FLinearColor::White };

	// State To RootOverlay Opacity
	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Build|Image|Opacity")
	float				DisableOpacity = { 0.2f };
};
