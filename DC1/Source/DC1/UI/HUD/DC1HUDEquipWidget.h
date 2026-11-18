// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1HUDEquipWidget.generated.h"

class UDC1EquipViewModel;
class UImage;
class UDC1DigitCounter;
class UWidgetSwitcher;

/**
 * HUD의 장비 섹션. 무기 슬롯과 팰스피어 보유 상황을 표시한다.
 * ViewModel은 HUD를 거치지 않고 이 위젯이 직접 조회한다.
 */
UCLASS()
class DC1_API UDC1HUDEquipWidget : public UDC1UserWidget
{
	GENERATED_BODY()

private:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindViewModel();
	void UnbindViewModel();

private:
	void HandleWeaponDisplayChanged();
	void HandleAmmoDisplayChanged();
	void HandleSphereDisplayChanged();

private:
	UPROPERTY(Transient, meta=(BindWidget))
	TObjectPtr<UImage>					Image_Icon_Weapon = {};

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1DigitCounter>		Digit_PalSphere = {};

	// 고른 스피어 등급(2 키) 아이콘
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage>					Image_Icon_PalSphere = {};

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1DigitCounter>		Digit_WeaponCurrent = {};

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1DigitCounter>		Digit_WeaponRemain = {};

	// [0] 무한대 / [1] 잔탄 카운터. 탄약 개념이 없는 무기는 0
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher>		WS_DigitWeapon = {};

private:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D>				Texture_Default = {};

private:
	TWeakObjectPtr<UDC1EquipViewModel> EquipViewModel = { nullptr };
};