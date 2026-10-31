// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Notification/DC1NotificationWidget.h"

#include "Subsystem/DC1ViewModelSubsystem.h"
#include "UI/Pal/DC1PalCaptureNoticeWidget.h"

void UDC1NotificationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 입력을 받지 않는다
	SetVisibility(ESlateVisibility::HitTestInvisible);

	CaptureNotice->OnNoticeFinished.AddUObject(this, &ThisClass::HandleCaptureNoticeFinished);

	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		NotificationViewModel = VMSubsystem->GetViewModel<UDC1NotificationViewModel>();
	}

	if (true == NotificationViewModel.IsValid())
	{
		CaptureNoticeHandle = NotificationViewModel->OnPalCaptureNotice.AddUObject(this, &ThisClass::HandlePalCaptureNotice);
	}
}

void UDC1NotificationWidget::NativeDestruct()
{
	if (true == NotificationViewModel.IsValid())
	{
		NotificationViewModel->OnPalCaptureNotice.Remove(CaptureNoticeHandle);
	}

	CaptureNotice->OnNoticeFinished.RemoveAll(this);

	Super::NativeDestruct();
}

void UDC1NotificationWidget::HandlePalCaptureNotice(const FDC1PalCaptureNoticeInfo& InInfo)
{
	PendingCaptureNotices.Add(InInfo);

	if (false == bShowingCaptureNotice)
	{
		ShowNextCaptureNotice();
	}
}

void UDC1NotificationWidget::HandleCaptureNoticeFinished()
{
	bShowingCaptureNotice = false;

	ShowNextCaptureNotice();
}

void UDC1NotificationWidget::ShowNextCaptureNotice()
{
	if (true == PendingCaptureNotices.IsEmpty())
	{
		return;
	}

	const FDC1PalCaptureNoticeInfo Info = PendingCaptureNotices[0];
	PendingCaptureNotices.RemoveAt(0);

	bShowingCaptureNotice = true;
	CaptureNotice->ShowNotice(Info);
}
