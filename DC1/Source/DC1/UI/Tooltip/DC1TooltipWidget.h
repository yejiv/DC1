// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "GameplayTagContainer.h"
#include "DC1TooltipWidget.generated.h"

class UTextBlock;
class UDynamicEntryBox;

UCLASS()
class DC1_API UDC1TooltipWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	static FText	FindTagName(const TMap<FGameplayTag, FText>& InTagNames, const FGameplayTag& InTag);

protected:
	void			ResetMaterials();
	bool			AddMaterial(FName InItemID, int32 InRequiredCount);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock>			Text_Name = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>			Text_Description = { nullptr };

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UDynamicEntryBox>	MaterialListBox = { nullptr };
};
