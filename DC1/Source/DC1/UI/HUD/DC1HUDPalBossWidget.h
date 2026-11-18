// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1HUDPalBossWidget.generated.h"

class UDC1PalBossViewModel;
class UDC1StatusBar;
class UImage;
class UTextBlock;

/**
 * HUD 의 보스 HP바
 */
UCLASS()
class DC1_API UDC1HUDPalBossWidget : public UDC1UserWidget
{
	GENERATED_BODY()

private:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindViewModel();
	void UnbindViewModel();

	// 보스가 바뀌거나 사라짐
	void HandleBossChanged();

	void HandleBossStatChanged();

	void SetElementIcon(UImage* InImage, UTexture2D* InIcon);

private:
	TWeakObjectPtr<UDC1PalBossViewModel> BossViewModel = { nullptr };

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Title = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>			Image_Element0 = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>			Image_Element1 = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Level = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Name = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1StatusBar>	HealthBar = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Health = { nullptr };
};
