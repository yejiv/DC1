// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1TitleWidget.generated.h"

class UWidgetSwitcher;
class UDC1TitleButton;
class UDC1BracketButton;
class UEditableTextBox;
class UDC1SoundOptionWidget;

UENUM(BlueprintType)
enum class EDC1Title : uint8
{
	Main = 0 UMETA(DisplayName = "Main"),
	Connect = 1 UMETA(DisplayName = "Connect"),
	Option = 2 UMETA(DisplayName = "Option")
};

UCLASS()
class DC1_API UDC1TitleWidget : public UDC1UserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void	NativeOnInitialized() override;
	virtual void	NativeConstruct() override;
	virtual void	NativeOnDeactivated() override;
	virtual bool	ShouldPropagateTo(const UDC1UserWidget* InChild) const override;

private:
	void			OnStartClicked();
	void			OnQuitClicked();
	void			OnReturnClicked();
	void			OnOptionClicked();
	void			OnOptionReturnClicked();

	UFUNCTION()
	void			OnConnectClicked();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher>		MenuSwitcher = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1TitleButton>		StartButton = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1TitleButton>		QuitButton = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox>	IPTextBox = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1BracketButton>	ConnectButton = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1TitleButton>		ReturnButton = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1TitleButton>		OptionButton = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1TitleButton>		OptionReturnButton = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1SoundOptionWidget>	SoundOptionWidget = { nullptr };
};
