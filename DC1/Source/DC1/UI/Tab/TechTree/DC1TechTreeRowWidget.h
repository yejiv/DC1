// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1TechTreeRowWidget.generated.h"

class UTextBlock;

UCLASS()
class DC1_API UDC1TechTreeRowWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void InitTier(int32 Level);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelText;
};
