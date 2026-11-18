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
	void					InitTier(int32 InLevel);
	UDC1TechTreeButton*		AddTechSlot(const FDC1TechRow& InRow);

	void					SetState(int32 InNewLevel);

private:
	void					ApplyState(bool InIsActivated, bool InIsActivatedTopLine, bool InIsActivatedBottomLine);

private:
	// Level Text
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>				LevelText = { nullptr };

	// Level Icon Image
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>					Level_Icon = { nullptr };

	// Level Icon Background
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>					Level_Background = { nullptr };

	// 비활성화된 Row Hover Image ( Blend Black )
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>					Level_HoverImage = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>					Row_Background = { nullptr };

	// 활성화된 Row Level Icon 간 Line
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>					Level_TopLine = { nullptr };

	// 활성화된 Row Level Icon 간 Line
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>					Level_BottomLine = { nullptr };

	// Slot Container
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>			SlotContainer = { nullptr };

	// 생성할 Slot Class
	UPROPERTY(EditAnywhere, Category = "TechTree")
	TSubclassOf<UDC1TechTreeButton>		SlotWidgetClass = { nullptr };

	UPROPERTY(EditAnywhere, Category = "TechTree|Image")
	float								InactiveHoverOpacity = { 0.5f };

	UPROPERTY(EditAnywhere, Category = "TechTree|Image")
	FLinearColor						ActiveIconColor = { FLinearColor::Blue };

	UPROPERTY(EditAnywhere, Category = "TechTree|Image")
	FLinearColor						InactiveIconColor = { FLinearColor::White };

	UPROPERTY(EditAnywhere, Category = "TechTree|Image")
	FLinearColor						ActiveBackgroundColor = { FLinearColor::White };

	UPROPERTY(EditAnywhere, Category = "TechTree|Image")
	FLinearColor						InactiveBackgroundColor = { FLinearColor::Black };

private:
	int32								RowLevel = { 0 };
};
