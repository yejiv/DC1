// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1CustomizeWidget.generated.h"

class UDC1CustomizeViewModel;
class UDC1CustomizeMenuButton;
class UDC1UserButton;
class UWidgetSwitcher;

/**
 * 커스터마이징 화면. 타이틀(최초 진입) / 게임 중 재커스터마이징 양쪽에서
 * UI_Customize 태그로 동일하게 Push된다.
 *
 * 좌측 메뉴 버튼(Head/Hair)은 MenuButtons 배열로 관리한다 - 하나가 클릭되면 나머지를 Inactive로 돌려서
 * 상호 배타적 선택을 유지한다. 게임 시작(확정) 버튼은 메뉴 선택 개념이 아니라서 배열에 안 넣고 따로 관리한다.
 *
 * ContentSwitcher의 자식 순서는 EDC1CustomizeMenu 순서(Head=0, Hair=1)와 반드시 맞춰야 한다 -
 * 메뉴 버튼 클릭 시 GetMenuType()을 인덱스로 그대로 캐스팅해서 스위칭하기 때문.
 */
UCLASS()
class DC1_API UDC1CustomizeWidget : public UDC1UserWidget
{
	GENERATED_BODY()

private:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindViewModel();
	void UnbindViewModel();

	void SetupMenuButtons();

	void HandleMenuButtonClicked(UDC1CustomizeMenuButton* ClickedButton);
	void HandleStartGameClicked();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1CustomizeMenuButton>	HeadMenuButton = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1CustomizeMenuButton>	HairMenuButton = { nullptr };

	// 메뉴 선택이 아니라 확정 액션이라 MenuButtons 배열과 별도로 관리
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1UserButton>				StartGameButton = { nullptr };

	// 선택된 메뉴에 맞는 콘텐츠 페이지로 전환
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher>				ContentSwitcher = { nullptr };

private:
	UPROPERTY()
	TArray<TObjectPtr<UDC1CustomizeMenuButton>>	MenuButtons;

	TWeakObjectPtr<UDC1CustomizeViewModel>	CustomizeViewModel = { nullptr };
};
