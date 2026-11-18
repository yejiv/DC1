// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1HUDBaseInfoWidget.generated.h"

class UDC1BaseInfoViewModel;
class UDynamicEntryBox;
class UTextBlock;

/**
 * HUD 의 거점 정보. 거점 범위 안에서만 보인다.
 * 거점 팰 수 / 정원 + 팰마다 한 줄 ( 아이콘 · 일하는 중 망치 · 상태 문구 )
 */
UCLASS()
class DC1_API UDC1HUDBaseInfoWidget : public UDC1UserWidget
{
	GENERATED_BODY()

private:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindViewModel();
	void UnbindViewModel();

	void HandleBaseInfoChanged();

private:
	TWeakObjectPtr<UDC1BaseInfoViewModel> BaseInfoViewModel = { nullptr };

private:
	// "3/5"
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>				Text_PalCount = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox>		RowContainer = { nullptr };

private:
	// 상태 문구 ( EDC1BasePalCondition )
	UPROPERTY(EditAnywhere, Category = "BaseInfo")
	FText								HealthyText;

	UPROPERTY(EditAnywhere, Category = "BaseInfo")
	FText								TiredText;

	UPROPERTY(EditAnywhere, Category = "BaseInfo")
	FText								RestingText;

	UPROPERTY(EditAnywhere, Category = "BaseInfo")
	FText								CapableText;
};
