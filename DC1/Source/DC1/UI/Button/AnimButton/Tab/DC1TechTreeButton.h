// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/AnimButton/DC1BracketButton.h"
#include "Enum/DC1Enums.h"
#include "DC1TechTreeButton.generated.h"

struct FDC1TechRow;

UCLASS()
class DC1_API UDC1TechTreeButton : public UDC1BracketButton
{
	GENERATED_BODY()
	
public:
	const FName&	GetTechID() const { return TechID; }

protected:
	virtual void	NativeOnInitialized() override;

protected:
	virtual void	HandleHovered() override;
	virtual void	HandleUnhovered() override;

public:
	void			InitTechButton(const FDC1TechRow& Row);
	void			SetState(EDC1TechSlotState NewState);

private:
	void			ApplyState();

private:
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation>	UnlockedAnim;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		LockedImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		AvailableImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		TechIconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		IconBackImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		TechNameBackImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		ButtonHoverImage;

	//COST Text = BracketButton::TextLabel

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>	TechNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>	CategoryNameText;

	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Font")
	FLinearColor			AvailableFontColor = FLinearColor::Blue;

	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Font")
	FLinearColor			UnlockedFontColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	FLinearColor			UnlockedIconBackImageColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	FLinearColor			AvailableIconBackImageColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	FLinearColor			UnlockedTechNameBackColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	FLinearColor			LockedTechNameBackColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	float					LockedImageOpacity = 0.8f;

	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	float					AvailableImageOpacity = 0.5f;

	FName				TechID = NAME_None;
	EDC1TechSlotState	SlotState = EDC1TechSlotState::Default;
};
