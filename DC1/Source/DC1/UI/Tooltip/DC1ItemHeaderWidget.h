// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "GameplayTagContainer.h"
#include "DC1ItemHeaderWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class DC1_API UDC1ItemHeaderWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void	SetItem(FName InItemID);

private:
	void	SetStatLine(UPanelWidget* InLineBox, UTextBlock* InValueText, int32 InValue);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>			Image_Icon = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>			Image_HeaderBack = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Name = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_ItemType = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_ItemRarity = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>	AttackBox = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Attack = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>	DefenseBox = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Defense = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>	HealthBox = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Health = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>	SatietyBox = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Satiety = { nullptr };

	UPROPERTY(EditAnywhere, Category = "Tooltip|Item", meta = (Categories = "Item.Type"))
	TMap<FGameplayTag, FText>	ItemTypeNamesByTag;
};
