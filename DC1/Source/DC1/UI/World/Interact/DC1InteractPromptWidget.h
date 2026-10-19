// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Interface/DC1InteractableInterface.h"
#include "DC1InteractPromptWidget.generated.h"

class UDynamicEntryBox;
class UDC1InteractPromptRowWidget;

UCLASS()
class DC1_API UDC1InteractPromptWidget : public UDC1UserWidget
{
	GENERATED_BODY()
	
public:
	void		SetOptions(const TArray<FDC1InteractOption>& InOptions);
	void		SetKeyHighlighted(EDC1InteractKey InKey, bool IsInHighlighted);
	void		SetHoldRatio(EDC1InteractKey InKey, float InRatio);

private:
	// 행 클래스 = Entry Widget Class, 행 간격 = Entry Spacing Y ( WBP 디테일 )
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox>			PromptEntryBox = { nullptr };

	UPROPERTY()
	TMap<EDC1InteractKey, TObjectPtr<UDC1InteractPromptRowWidget>> RowWidgetByKey;
};
