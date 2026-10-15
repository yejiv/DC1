// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/World/BuildInfo/DC1BuildInfoWidget.h"
#include "DC1ConstructionInfoWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UWidgetSwitcher;

// 건설 패널 SwitcherIndex 매핑 키
UENUM()
enum class EDC1ConstructionInfoPanel : uint8
{
	NoWorker = 0,	// 작업자가 없습니다, 작업량 표시
	Working = 1,	// 건축 진행, 작업량 시간으로 표시 HH:MM:SS
};

UCLASS()
class DC1_API UDC1ConstructionInfoWidget : public UDC1BuildInfoWidget
{
	GENERATED_BODY()
	
protected:
	virtual void			Subscribe(ADC1InteractBuildActor* InBuildActor) override;
	virtual void			Unsubscribe(ADC1InteractBuildActor* InBuildActor) override;

	virtual void			RefreshValues() override;

private:
	// 초 -> HH:MM:SS
	static FText			FormatRemainingTime(int32 InSeconds);

	void					SetActivePanel(EDC1ConstructionInfoPanel InPanel);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher>				WorkerStateSwitcher;

	UPROPERTY(EditAnywhere, Category = "Panel")
	TMap<EDC1ConstructionInfoPanel, int32>	PanelToSwitchIndex;

	// No Work
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>					RemainingWorkText;

	// Working
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>					RemainingTimeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar>				BuildProgressBar;

	FDelegateHandle							WorkProgressChangedHandle;
};
