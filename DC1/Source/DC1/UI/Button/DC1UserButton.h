// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1UserButton.generated.h"

/**
 * 
 */
UCLASS()
class DC1_API UDC1UserButton : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	FSimpleMulticastDelegate OnClickedDelegate;

protected:
	virtual void NativeConstruct() override;

protected:
	UFUNCTION()
	virtual void HandleClicked();

	UFUNCTION()
	virtual void HandleHovered();

	UFUNCTION()
	virtual void HandleUnhovered();


protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton>				UserButton;
};
