// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Interface/DC1InteractableInterface.h"
#include "DC1HUDDownedWidget.generated.h"

class UDC1StatusBar;
class UDC1InteractPromptRowWidget;
class UDC1PlayerStatusViewModel;

/**
 * HUD의 기절 섹션. 기절 게이지와 포기 안내를 표시한다.
 * ScreenStack에 올라가지 않으므로 기절 IMC가 유지된다. 기절 여부에 따라 스스로 Visible을 바꾼다.
 */
UCLASS()
class DC1_API UDC1HUDDownedWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindViewModel();
	void UnbindViewModel();

private:
	void HandleDownedStateChanged(bool InIsDowned);
	void HandleDownedGaugeChanged(float Current, float Max);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1StatusBar>				DownedGaugeBar = { nullptr };

	// 포기 안내 ( 상호작용 프롬프트 행 재사용 )
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1InteractPromptRowWidget>	GiveUpPrompt = { nullptr };

	// 포기 프롬프트에 표시할 키 / 텍스트 ( ex) F : 포기 )
	UPROPERTY(EditDefaultsOnly, Category = "Downed")
	FDC1InteractOption						GiveUpOption;

private:
	TWeakObjectPtr<UDC1PlayerStatusViewModel> StatusViewModel = { nullptr };
};
