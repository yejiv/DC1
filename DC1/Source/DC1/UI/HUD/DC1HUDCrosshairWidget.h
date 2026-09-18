// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1HUDCrosshairWidget.generated.h"

class UDC1CrosshairViewModel;
class UImage;
class UWidgetAnimation;

/**
 * 화면 중앙 조준점. HUD 하위에 상시 존재하고 Visibility만 토글한다.
 * 매 발사마다 껐다 켜지므로 생성/파괴하지 않는다.
 *
 * 표시 조건 판정과 반동 시점 판정은 모두 ViewModel이 하고, 이 위젯은 결과만 반영한다.
 */
UCLASS()
class DC1_API UDC1HUDCrosshairWidget : public UDC1UserWidget
{
	GENERATED_BODY()

private:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindViewModel();
	void UnbindViewModel();

private:
	void HandleVisibilityChanged(bool bInVisible);

	// 반동. Strength로 재생 속도나 애님 선택을 가를 수 있다
	void HandleFired(float InStrength);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>						Image_Crosshair = {};

	// 발사 시점에 1회 재생. 베이스의 PlayAnim을 쓴다
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation>			RecoilAnim = { nullptr };

private:
	TWeakObjectPtr<UDC1CrosshairViewModel>	CrosshairViewModel = { nullptr };
};
