// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "ViewModel/Notification/DC1NotificationViewModel.h"
#include "DC1FeedNoticeRowWidget.generated.h"

class UImage;
class UTextBlock;
class UDC1FeedNoticeRowWidget;

DECLARE_DELEGATE_OneParam(FOnFeedNoticeRowFinished, UDC1FeedNoticeRowWidget*);

/**
 * 왼쪽 알림 목록의 한 줄 (아이콘 + 문구). IntroAnim → DisplayDuration 동안 유지 → OutroAnim → 끝 알림
 * 엔트리 박스가 위젯을 재사용하므로 목록에서 뺄 때는 StopNotice 로 타이머를 끊는다
 */
UCLASS()
class DC1_API UDC1FeedNoticeRowWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void						ShowNotice(const FDC1FeedNoticeInfo& InInfo);

	// 다 사라지기 전에 목록에서 뺄 때
	void						StopNotice();

public:
	// 다 사라졌다. 알림 층이 목록에서 뺀다
	FOnFeedNoticeRowFinished	OnRowFinished;

protected:
	virtual void				NativeDestruct() override;
	virtual void				OnAnimFinished(const UWidgetAnimation* InAnim) override;

private:
	void						BeginHide();

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>			Image_Icon = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Message = { nullptr };

	// 다 나타난 뒤 떠 있는 시간
	UPROPERTY(EditAnywhere, Category = "Notice", meta = (ClampMin = "0.0"))
	float						DisplayDuration = { 4.f };

	FTimerHandle				HideTimer;
};
