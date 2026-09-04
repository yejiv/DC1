// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1StaminaWidget.generated.h"

class UImage;
class UDC1StatusBar;
class UDC1PlayerStatusViewModel;

/**
 * 플레이어 옆에 월드로 띄우는 스태미나 표시.
 * WidgetComponent가 소유하므로 HUD 트리에 들어가지 않는다.
 *
 * 게이지 자체는 UDC1StatusBar를 그대로 재사용하고,
 * 이 클래스는 ViewModel 연결과 상태 아이콘 교체만 담당한다.
 */
UCLASS()
class DC1_API UDC1StaminaWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindViewModel();
	void UnbindViewModel();

private:
	void HandleStaminaChanged(float Current, float Max);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1StatusBar>	StaminaBar = { nullptr };

	// 사람 / 팰 탑승에 Visible On/Off
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage>			Image_Icon = { nullptr };

private:
	TWeakObjectPtr<UDC1PlayerStatusViewModel> StatusViewModel = { nullptr };

private:
	
};