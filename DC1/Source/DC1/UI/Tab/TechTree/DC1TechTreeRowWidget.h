// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1TechTreeRowWidget.generated.h"

struct FDC1TechRow;

class UImage;
class UTextBlock;
class UDC1TechTreeButton;

UCLASS()
class DC1_API UDC1TechTreeRowWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void					InitTier(int32 Level);
	UDC1TechTreeButton*		AddTechSlot(const FDC1TechRow& Row);

	void					SetState(int32 NewLevel);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>				LevelText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>					Level_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>					Level_Background;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>					Level_HoverImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>					Row_Background;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>					Level_TopLine;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>					Level_BottomLine;

	// Slot Container
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>			SlotContainer;

	// 생성할 Slot Class
	UPROPERTY(EditAnywhere, Category = "TechTree")
	TSubclassOf<UDC1TechTreeButton>		SlotWidgetClass;

	UPROPERTY(EditAnywhere, Category = "TechTree|Image")
	float								InactiveHoverOpacity = 0.5f;

	UPROPERTY(EditAnywhere, Category = "TechTree|Image")
	FLinearColor						ActiveIconColor = FLinearColor::Blue;

	UPROPERTY(EditAnywhere, Category = "TechTree|Image")
	FLinearColor						InactiveIconColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "TechTree|Image")
	FLinearColor						ActiveBackgroundColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "TechTree|Image")
	FLinearColor						InactiveBackgroundColor = FLinearColor::Black;

	int32								RowLevel = 0;
};
