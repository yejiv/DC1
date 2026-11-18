// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1GameMenuWidget.generated.h"

class UDC1UserButton;

UCLASS()
class DC1_API UDC1GameMenuWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void	NativeOnInitialized() override;
	virtual void	NativeOnDeactivated() override;
	virtual bool	ShouldPropagateTo(const UDC1UserWidget* InChild) const override;

private:
	void			HandleOptionClicked();
	void			HandleTitleClicked();

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1UserButton>	Button_Option;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1UserButton>	Button_Title;

	// 사운드 변경
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1UserWidget>	OptionWidget;

};
