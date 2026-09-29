// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/DC1UserButton.h"
#include "ViewModel/Pal/DC1PalBoxDisplayTypes.h"
#include "DC1PalBoxSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UWidgetAnimation;
class UDC1StatusBar;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPalBoxSlotHovered, int32 /*SlotIndex*/);

/**
 * 보유 팰 한 칸
 */
UCLASS()
class DC1_API UDC1PalBoxSlotWidget : public UDC1UserButton
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

protected:
	virtual void	HandleHovered() override;
	virtual void	HandleUnhovered() override;

private:
	// 빈 칸이면 숨김
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidget>			Panel_Content = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Level = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Name = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1StatusBar>	HealthBar = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Health = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_MaxHealth = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>			Image_Icon = { nullptr };

	// 브래킷과 테두리. 호버 동안만 보인다
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidget>			Panel_Hover = { nullptr };

	// 카드 색 전환. 호버 시 정방향, 언호버 시 역방향
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation>	HoverInAnim = { nullptr };

	// 브래킷 크기 반복. 호버 동안 무한 루프
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation>	HoverLoopAnim = { nullptr };

private:
	int32						SlotIndex = { INDEX_NONE };
};
