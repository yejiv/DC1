// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/AnimButton/DC1BracketButton.h"
#include "GameplayTagContainer.h"
#include "DC1TabHeaderButton.generated.h"

UCLASS()
class DC1_API UDC1TabHeaderButton : public UDC1BracketButton
{
	GENERATED_BODY()

public:
	const FGameplayTag& GetTabTag() { return TabTag; }
	//UUserWidget
protected:
	virtual void	NativeConstruct() override;

protected:
	virtual void	HandleHovered() override;
	virtual void	HandleUnhovered() override;
public:
	void			SetActive(bool IsActive);

private:
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation>	ClickedAnim;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>	ActiveBackgroundImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>	HoverImage;

	UPROPERTY(EditAnywhere)
	FGameplayTag		TabTag = FGameplayTag();

	UPROPERTY(EditAnywhere)
	FLinearColor		ActiveColor = FLinearColor::White;

	UPROPERTY(EditAnywhere)
	FLinearColor		InactiveColor = FLinearColor::White;

	UPROPERTY(EditAnywhere)
	float				HoverOpacity = 0.4f;
};
