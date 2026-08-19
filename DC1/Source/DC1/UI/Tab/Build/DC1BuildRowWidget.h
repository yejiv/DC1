// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "GameplayTagContainer.h"
#include "DC1BuildRowWidget.generated.h"

struct FDC1BuildRow;
struct FDC1TechRow;

class UDC1BuildButton;
class UTextBlock;

UCLASS()
class DC1_API UDC1BuildRowWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void		NativeOnInitialized() override;

public:
	void				InitRow(const FGameplayTag& InRowTag, const FText& RowName);
	UDC1BuildButton*	AddBuildSlot(const FDC1BuildRow& InBuildRow, const FDC1TechRow& InTechRow);

	bool				VisibleSlot(const FGameplayTag& InSlotTag);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>		SlotContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>			RowNameText;

	UPROPERTY(EditAnywhere, Category = "Build|Slot")
	TSubclassOf<UDC1BuildButton>	SlotWidgetClass;

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UDC1BuildButton>> Slots;

private:
	FGameplayTag	RowTag;
};