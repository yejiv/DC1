// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/AnimButton/DC1BracketButton.h"
#include "Enum/DC1Enums.h"
#include "GameplayTagContainer.h"
#include "DC1TechTreeButton.generated.h"

struct FDC1TechRow;

UCLASS()
class DC1_API UDC1TechTreeButton : public UDC1BracketButton
{
	GENERATED_BODY()
	
public:
	const FGameplayTag&	GetTechTag() const { return TechTag; }

protected:
	virtual void	NativeOnInitialized() override;

protected:
	virtual void	HandleHovered() override;
	virtual void	HandleUnhovered() override;

public:
	void			InitTechButton(const FDC1TechRow& InRow);
	void			SetState(EDC1TechSlotState InNewState);

private:
	void			ApplyState();

private:
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation>	UnlockedAnim = { nullptr };

	// Opacity 조절용 Overlay
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>	RootOverlay = { nullptr };

	// Lock 상태에서의 Hover Image
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>			LockedImage = { nullptr };

	// Available 상태에서의 Hover Image
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>			AvailableImage = { nullptr };

	// Icon Image
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>			TechIconImage = { nullptr };

	// Icon Background Image
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>			IconBackImage = { nullptr };

	// Tech Name Background Image ( ex) 원시적인 작업대 Background Image )
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>			TechNameBackImage = { nullptr };

	// Button Outline Image
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>			ButtonImage = { nullptr };

	//COST Text = BracketButton::TextLabel

	// Tech Name ( ex) 원시적인 작업대 )
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		TechNameText = { nullptr };

	// Category Name ( ex) 건축물 )
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		CategoryNameText = { nullptr };


#pragma region STATE TO TECH NAME FONT COLOR
	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Font")
	FLinearColor				AvailableFontColor = { FLinearColor::Blue };

	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Font")
	FLinearColor				UnlockedFontColor = { FLinearColor::White };
#pragma endregion



#pragma region STATE TO BUTTON OUTLINE IMAGE

	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	FSlateBrush					LockedButtonImageBrush;
	
	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	FSlateBrush					AvailableButtonImageBrush;
	
	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	FSlateBrush					UnlockedButtonImageBrush;


	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	FSlateBrush					HoverAvailableButtonImageBrush;

#pragma endregion




#pragma region STATE TO ICON BACKGROUND IMAGE COLOR
	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	FLinearColor				UnlockedIconBackImageColor = { FLinearColor::White };

	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	FLinearColor				AvailableIconBackImageColor = { FLinearColor::White };
#pragma endregion



#pragma region STATE TO TECH NAME BACKGROUND COLOR
	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	FLinearColor				UnlockedTechNameBackColor = { FLinearColor::White };

	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	FLinearColor				LockedTechNameBackColor = { FLinearColor::White };
#pragma endregion



#pragma region STATE TO OPACITY
	// Available 상태에서 Hover Image Opacity
	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	float						AvailableImageOpacity = { 0.5f };

	// Lock 상태에서의 모든 이미지 Opacity 비율 ( RootOverlay Opacity )
	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	float						LockedOpacity = { 0.3f };

	// Lock 상태에서 Hover Image Opacity
	UPROPERTY(EditAnywhere, Category = "UserButton|Tech|Image")
	float						LockedImageOpacity = { 0.7f };
#pragma endregion


private:
	int32						RequiredLevel = { 0 };
	FGameplayTag				TechTag = { FGameplayTag::EmptyTag };
	EDC1TechSlotState			SlotState = { EDC1TechSlotState::Default };
};
