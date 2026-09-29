// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/DC1UserButton.h"
#include "UI/Pal/PalBox/DC1PalBoxSlotWidget.h"
#include "DC1PalBoxStorageSlotWidget.generated.h"

class UImage;
class UWidgetAnimation;

/**
 * 상자 한 칸
 */
UCLASS()
class DC1_API UDC1PalBoxStorageSlotWidget : public UDC1UserButton
{
	GENERATED_BODY()

public:
	void			SetSlotIndex(int32 InSlotIndex) { SlotIndex = InSlotIndex; }
	void			SetSlotInfo(const FDC1PalBoxSlotInfo& InSlotInfo);
	void			SetEmpty();

public:
	FOnPalBoxSlotHovered	OnSlotHovered;

protected:
	virtual void	NativeConstruct() override;
	virtual void	HandleHovered() override;
	virtual void	HandleUnhovered() override;

private:
	// 빈 칸이면 숨김
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidget>			Panel_Content = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>			Image_Icon = { nullptr };

	// 호버 테두리
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget>			Panel_Hover = { nullptr };

	// 브래킷 크기 반복. 호버 동안 무한 루프
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation>	HoverLoopAnim = { nullptr };

private:
	int32						SlotIndex = { INDEX_NONE };
};
