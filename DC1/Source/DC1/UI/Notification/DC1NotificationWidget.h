// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "ViewModel/Notification/DC1NotificationViewModel.h"
#include "DC1NotificationWidget.generated.h"

class UDC1PalCaptureNoticeWidget;

/**
 * 알림 층. HUD 처럼 UIManager 의 오버레이로 항상 떠 있고 입력을 받지 않는다
 * 알림은 대기열로 하나씩 보여 준다
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

private:
	// 우하단 포획 알림. WBP 에 하나 배치해 두고 재사용한다
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1PalCaptureNoticeWidget>	CaptureNotice = { nullptr };

	TArray<FDC1PalCaptureNoticeInfo>		PendingCaptureNotices;
	bool									bShowingCaptureNotice = { false };

	TWeakObjectPtr<UDC1NotificationViewModel>	NotificationViewModel;
	FDelegateHandle							CaptureNoticeHandle;
};
