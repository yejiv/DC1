#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1PlayerStatPanelWidget.generated.h"

class UButton;
class UTextBlock;
class UDC1PlayerStatWidget;
class UDC1PlayerStatsViewModel;

// 인벤토리 스탯 패널 (남은 포인트, 스탯 6줄, 강화 모드 확정/취소)
UCLASS()
class DC1_API UDC1PlayerStatPanelWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void SetEnhanceMode(bool bInEnhanceMode);
	void RefreshButtons();
	void HandleRemainingStatPointsChanged(int32 InRemainingPoints);

	UFUNCTION()
	void HandleEnhanceStatsClicked();

	UFUNCTION()
	void HandleConfirmClicked();

	UFUNCTION()
	void HandleCancelClicked();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_StatPoints;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1PlayerStatWidget> PlayerStat_Health;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1PlayerStatWidget> PlayerStat_Stamina;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1PlayerStatWidget> PlayerStat_Attack;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1PlayerStatWidget> PlayerStat_Defense;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1PlayerStatWidget> PlayerStat_WorkSpeed;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1PlayerStatWidget> PlayerStat_Weight;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_EnhanceStats;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Confirm;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Cancel;

	UPROPERTY()
	TArray<TObjectPtr<UDC1PlayerStatWidget>> StatWidgets;

	UPROPERTY()
	TObjectPtr<UDC1PlayerStatsViewModel> CachedViewModel = { nullptr };

	bool bEnhanceMode = { false };
};
