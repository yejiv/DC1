#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1PlayerStatusPanelWidget.generated.h"

class UHorizontalBox;
class UProgressBar;
class UTextBlock;
class UDC1MenuGaugeWidget;
class UDC1PlayerStatusViewModel;

// 인벤토리 상태 열 위쪽 (레벨 박스, 실드/HP/배고픔 게이지, 버프 아이콘 줄)
UCLASS()
class DC1_API UDC1PlayerStatusPanelWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindViewModel();
	void UnbindViewModel();

	void HandleHealthChanged(float InCurrent, float InMax);
	void HandleShieldChanged(float InCurrent, float InMax);
	void HandleSatietyChanged(float InCurrent, float InMax);
	void HandleLevelChanged(int32 InLevel, int32 InRemainingExp, float InExpPercent);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Level;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_NextExp;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ProgressBar_Exp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1MenuGaugeWidget> MenuGauge_Shield;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1MenuGaugeWidget> MenuGauge_Health;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1MenuGaugeWidget> MenuGauge_Satiety;

	// 버프 시스템이 생기면 아이콘을 채울 자리
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HorizontalBox_Buffs;

	UPROPERTY()
	TObjectPtr<UDC1PlayerStatusViewModel> CachedViewModel = { nullptr };
};
