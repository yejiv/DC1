// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "ViewModel/Notification/DC1NotificationViewModel.h"
#include "DC1PalCaptureNoticeWidget.generated.h"

class UDynamicEntryBox;
class UImage;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE(FOnPalCaptureNoticeFinished);

/**
 * 포획 성공 알림 한 건. IntroAnim → DisplayDuration 동안 유지 → OutroAnim → 숨김
 */
UCLASS()
class DC1_API UDC1PalCaptureNoticeWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void			ShowNotice(const FDC1PalCaptureNoticeInfo& InInfo);

public:
	// 다 사라졌다. 알림 층이 다음 알림을 꺼낸다
	FOnPalCaptureNoticeFinished	OnNoticeFinished;

protected:
	virtual void	NativeOnInitialized() override;
	virtual void	NativeDestruct() override;
	virtual void	OnAnimFinished(const UWidgetAnimation* InAnim) override;

private:
	void			BeginHide();
	void			FinishHide();

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>				Image_Icon = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>			Text_Name = { nullptr };

	// 숫자만. "LV" 글자는 WBP 에 고정
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>			Text_Level = { nullptr };

	// 구분선 + 패시브 목록. 패시브가 없으면 통째로 숨긴다
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidget>				Panel_Passive = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox>	PassiveContainer = { nullptr };

	// 다 나타난 뒤 떠 있는 시간
	UPROPERTY(EditAnywhere, Category = "Notice", meta = (ClampMin = "0.0"))
	float							DisplayDuration = { 3.f };

	FTimerHandle					HideTimer;
};
