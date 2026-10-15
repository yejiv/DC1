// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/AnimButton/DC1BracketButton.h"
#include "GameplayTagContainer.h"
#include "Define/DC1Enums.h"
#include "DC1TabHeaderButton.generated.h"

UCLASS()
class DC1_API UDC1TabHeaderButton : public UDC1BracketButton
{
	GENERATED_BODY()

public:
	const FGameplayTag& GetTabTag() const { return TabTag; }
	bool				CanSelect() const { return ((ButtonState != EDC1TabButtonState::Disable) && (ButtonState != EDC1TabButtonState::Default)); }
	
protected:
	virtual void	NativeConstruct() override;

protected:
	virtual void	HandleHovered() override;
	virtual void	HandleUnhovered() override;

	virtual void	ApplyState();

	// 애님이 끝나면 상태 기준으로 다시 확정한다
	virtual void	OnAnimFinished(const UWidgetAnimation* InAnim) override;

public:
	virtual bool	SetState(EDC1TabButtonState InState);

protected:
	EDC1TabButtonState	ButtonState = { EDC1TabButtonState::Default };

private:
	// ActiveBackgroundImage Anim 
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation>	ClickedAnim = { nullptr };

	// Anim Image
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage>	ActiveBackgroundImage = { nullptr };

	// HOVER IMAGE OPTIONAL
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage>	HoverImage = { nullptr };

	// Tab 활성화 Button Color
	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Image")
	FLinearColor		ActiveColor = { FLinearColor::White };

	// Tab 비활성화 Button Color
	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Image")
	FLinearColor		InactiveColor = { FLinearColor::White };

	// Tab 선택 불가 Button Color
	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Image")
	FLinearColor		DisableColor = { FLinearColor::White };

	// Tab Widget에서 Index 캐싱 용 Tag
	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Image")
	FGameplayTag		TabTag = { FGameplayTag::EmptyTag };

	// HOVER IMAGE OPTIONAL - Opacity
	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Image")
	float				HoverOpacity = { 0.4f };

	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Image")
	float				UnhoverOpacity = { 0.f };
};
