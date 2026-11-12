// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/World/BuildInfo/Production/DC1ProductionInfoWidget.h"
#include "DC1FarmInfoWidget.generated.h"

class UTextBlock;

UCLASS()
class DC1_API UDC1FarmInfoWidget : public UDC1ProductionInfoWidget
{
	GENERATED_BODY()

protected:
	virtual void			RefreshValues() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>			CurrentPhaseText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget>				NextPhasePanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>			NextPhaseText;
};
