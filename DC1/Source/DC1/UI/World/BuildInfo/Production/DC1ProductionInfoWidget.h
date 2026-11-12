// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/World/BuildInfo/DC1BuildInfoWidget.h"
#include "DC1ProductionInfoWidget.generated.h"

class UImage;
class UTextBlock;
class UWidgetSwitcher;
class UWidgetAnimation;

UCLASS()
class DC1_API UDC1ProductionInfoWidget : public UDC1BuildInfoWidget
{
	GENERATED_BODY()
	
protected:
	virtual void			OnAnimFinished(const UWidgetAnimation* InAnim) override;
	virtual void			Subscribe(ADC1InteractBuildActor* InBuildActor) override;
	virtual void			Unsubscribe(ADC1InteractBuildActor* InBuildActor) override;
	virtual void			RefreshValues() override;
	virtual void			OnInfoStateChanged(EDC1BuildInfoState InPrevState, EDC1BuildInfoState InNewState) override;

private:
	void					SetActivePanel(EDC1BuildInfoState InState);
	void					SetItemInfo(FName InItemID);
	void					SetDetailPanelVisible(bool InIsVisible);

private:
	// NoItem <-> 생산
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher>		ProductionStateSwitcher;

	UPROPERTY(EditAnywhere, Category = "Panel")
	TMap<EDC1BuildInfoState, int32>	StateToSwitchIndex;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				ItemProgressImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock>			StoredCountText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock>			MaxCountText;

	// Detail 전용 영역 ( 이름 + 작업자 )
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget>				DetailPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>			ItemNameText;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation>	ExpandAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Progress")
	FName							ItemProgressParamName = { "Percent" };

private:
	FDelegateHandle					ProductionChangedHandle;
	FName							CachedItemID = { NAME_None };
};
