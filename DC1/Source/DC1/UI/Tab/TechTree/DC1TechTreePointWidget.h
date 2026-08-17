// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Enum/DC1Enums.h"
#include "DC1TechTreePointWidget.generated.h"


class UImage;
class UTextBlock;

UCLASS()
class DC1_API UDC1TechTreePointWidget : public UDC1UserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void	NativeOnInitialized() override;
	virtual void	NativeDestruct() override;

public:
	void			SetPoint(int32 NumPoint);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				TechPoint_FrontBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				TechPoint_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>		TechPointOverlay;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				TechPoint_Background;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				TechPoint_Box;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>			TechPoint_Label;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>			TextPoint_Num;

	UPROPERTY(EditAnywhere, Category = "TechPoint|Overlay")
	float							NoPointOverlayOpacity = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = "TechPoint|Image")
	FSlateBrush						IconBrush;

	UPROPERTY(EditAnywhere, Category = "TechPoint|Image")
	FLinearColor					BoxColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "TechPoint|Image")
	FLinearColor					HasPointBackgroundColor = FLinearColor::White;
	
	UPROPERTY(EditAnywhere, Category = "TechPoint|Image")
	FLinearColor					NoPointBackgroundColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "TechPoint|Font")
	FText							TechPoint_LabelText;

	UPROPERTY(EditAnywhere, Category = "TechPoint|Font")
	FLinearColor					HasPointNumColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "TechPoint|Font")
	FLinearColor					NoPointNumColor = FLinearColor::White;

	EDC1TechPointState				PointState = EDC1TechPointState::Default;
};
