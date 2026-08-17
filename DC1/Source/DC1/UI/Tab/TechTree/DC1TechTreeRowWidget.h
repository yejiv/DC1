// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1TechTreeRowWidget.generated.h"

struct FDC1TechRow;
class UTextBlock;
class UDC1TechTreeButton;

UCLASS()
class DC1_API UDC1TechTreeRowWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void					InitTier(int32 Level);
	UDC1TechTreeButton*		AddTechSlot(const FDC1TechRow& Row);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>			SlotContainer;

	UPROPERTY(EditAnywhere, Category = "TechTree")
	TSubclassOf<UDC1TechTreeButton>		SlotWidgetClass;
};
