// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1TitleWidget.generated.h"

class UWidgetSwitcher;
class UDC1TitleButton;
class UDC1BracketButton;
class UEditableTextBox;

UENUM(BlueprintType)
enum class EDC1Title : uint8
{
	Main = 0 UMETA(DisplayName = "Main"),
	Connect = 1 UMETA(DisplayName = "Connect")
};

UCLASS()
class DC1_API UDC1TitleWidget : public UDC1UserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void	NativeOnInitialized() override;
	virtual void	NativeConstruct() override;

private:
	void			OnStartClicked();
	void			OnQuitClicked();
	void			OnReturnClicked();

	UFUNCTION()
	void			OnConnectClicked();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher>		MenuSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1TitleButton>		StartButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1TitleButton>		QuitButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox>	IPTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1BracketButton>	ConnectButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1TitleButton>		ReturnButton;
};
