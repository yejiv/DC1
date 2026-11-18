// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "ViewModel/Notification/DC1NotificationViewModel.h"
#include "DC1NotificationWidget.generated.h"

class UDC1PalCaptureNoticeWidget;
class UDC1FeedNoticeRowWidget;
class UDynamicEntryBox;

/**
 * 알림 층. HUD 처럼 UIManager 의 오버레이로 항상 떠 있고 입력을 받지 않는다
 * 포획 알림은 대기열로 하나씩 보여 준다
 * 한 줄 알림(획득·쓰러뜨림)은 왼쪽 목록에 아래로 쌓이고 각자 시간이 지나면 사라진다
 */
UCLASS()
class DC1_API UDC1NotificationWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void	NativeConstruct() override;
	virtual void	NativeDestruct() override;

private:
	void			HandlePalCaptureNotice(const FDC1PalCaptureNoticeInfo& InInfo);
	void			HandleCaptureNoticeFinished();

	void			ShowNextCaptureNotice();

	void			HandleFeedNotice(const FDC1FeedNoticeInfo& InInfo);
	void			RemoveFeedRow(UDC1FeedNoticeRowWidget* InRow);

private:
	// 우하단 포획 알림. WBP 에 하나 배치해 두고 재사용한다
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1PalCaptureNoticeWidget>	CaptureNotice = { nullptr };

	TArray<FDC1PalCaptureNoticeInfo>		PendingCaptureNotices;
	bool									bShowingCaptureNotice = { false };

	// 왼쪽 한 줄 알림 목록. 엔트리 클래스 = WBP_FeedNoticeRow. 새 줄은 맨 아래에 붙는다
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox>			FeedContainer = { nullptr };

	// 한꺼번에 보이는 최대 줄 수. 넘치면 가장 오래된 줄(맨 위)부터 뺀다
	UPROPERTY(EditAnywhere, Category = "Notice", meta = (ClampMin = "1"))
	int32									MaxFeedRows = { 5 };

	TWeakObjectPtr<UDC1NotificationViewModel>	NotificationViewModel;
	FDelegateHandle							CaptureNoticeHandle;
	FDelegateHandle							FeedNoticeHandle;
};
