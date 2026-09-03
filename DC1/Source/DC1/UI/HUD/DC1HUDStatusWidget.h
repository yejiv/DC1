// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1HUDStatusWidget.generated.h"

class UTextBlock;
class UDC1StatusBar;
class UDC1PlayerStatusViewModel;

/**
 * HUD의 상태 섹션. 체력을 표시한다.
 * ViewModel은 HUD를 거치지 않고 이 위젯이 직접 조회한다.
 */
UCLASS()
class DC1_API UDC1HUDStatusWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindViewModel();
	void UnbindViewModel();

private:
	void HandleHealthChanged(float Current, float Max);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1StatusBar> HealthBar = { nullptr };

	// "90 / 100" 형태의 수치 표기
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Health = { nullptr };
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_MaxHealth = { nullptr };

private:
	TWeakObjectPtr<UDC1PlayerStatusViewModel> StatusViewModel = { nullptr };
};