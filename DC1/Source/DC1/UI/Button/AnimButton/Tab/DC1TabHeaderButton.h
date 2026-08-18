// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/AnimButton/DC1BracketButton.h"
#include "GameplayTagContainer.h"
#include "Enum/DC1Enums.h"
#include "DC1TabHeaderButton.generated.h"

UCLASS()
class DC1_API UDC1TabHeaderButton : public UDC1BracketButton
{
	GENERATED_BODY()

public:
	const FGameplayTag& GetTabTag() const { return TabTag; }
	bool				CanSelect() const { return ((ButtonState != EDC1TabButtonState::Disable) && (ButtonState != EDC1TabButtonState::Default)); }
	//UUserWidget
protected:
	virtual void	NativeConstruct() override;

protected:
	virtual void	HandleHovered() override;
	virtual void	HandleUnhovered() override;

public:
	void			SetState(EDC1TabButtonState State);

private:
	void			ApplyState();

private:
	// ActiveBackgroundImage Anim 
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation>	ClickedAnim;

	// Anim Image
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage>	ActiveBackgroundImage;

	// Tab 활성화 Button Color
	UPROPERTY(EditAnywhere)
	FLinearColor		ActiveColor = FLinearColor::White;

	// Tab 비활성화 Button Color
	UPROPERTY(EditAnywhere)
	FLinearColor		InactiveColor = FLinearColor::White;

	// Tab 선택 불가 Button Color
	UPROPERTY(EditAnywhere)
	FLinearColor		DisableColor = FLinearColor::White;

	// Tab Widget에서 Index 캐싱 용 Tag
	UPROPERTY(EditAnywhere)
	FGameplayTag		TabTag = FGameplayTag();

	// HOVER IMAGE OPTIONAL
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage>	HoverImage;

	// HOVER IMAGE OPTIONAL - Opacity
	UPROPERTY(EditAnywhere)
	float				HoverOpacity = 0.4f;

private:
	EDC1TabButtonState	ButtonState = EDC1TabButtonState::Default;
};
