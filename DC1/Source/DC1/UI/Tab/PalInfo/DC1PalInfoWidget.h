// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1PalInfoWidget.generated.h"

class UWidgetSwitcher;
class UDC1PalViewModel;
class UDC1PalPartyWidget;
class UDC1PalDetailWidget;

/**
 * 팰 정보 화면
 * 팰 파티 현황(파티 팰 + 미리보기) / 팰 상세 정보 두 패널을 스위칭
 */
UCLASS()
class DC1_API UDC1PalInfoWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void SetViewModel(UDC1PalViewModel* InViewModel);

	void ShowPartyPanel();
	void ShowDetailPanel();

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher>		WidgetSwitcher_Panel = { nullptr };

	// 팰 파티 현황
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1PalPartyWidget>	PalPartyWidget = { nullptr };

	// 팰 상세 정보
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1PalDetailWidget>	PalDetailWidget = { nullptr };

	UPROPERTY()
	TObjectPtr<UDC1PalViewModel>	CachedViewModel = { nullptr };
};
