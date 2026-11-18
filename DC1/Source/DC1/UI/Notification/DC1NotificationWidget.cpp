// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Notification/DC1NotificationWidget.h"

#include "Subsystem/DC1ViewModelSubsystem.h"
#include "UI/Pal/DC1PalCaptureNoticeWidget.h"
#include "UI/Notification/DC1FeedNoticeRowWidget.h"

#include "Components/DynamicEntryBox.h"

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
		FeedNoticeHandle = NotificationViewModel->OnFeedNotice.AddUObject(this, &ThisClass::HandleFeedNotice);
	}
}

void UDC1NotificationWidget::NativeDestruct()
{
	if (true == NotificationViewModel.IsValid())
	{
		NotificationViewModel->OnPalCaptureNotice.Remove(CaptureNoticeHandle);
		NotificationViewModel->OnFeedNotice.Remove(FeedNoticeHandle);
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

void UDC1NotificationWidget::HandleFeedNotice(const FDC1FeedNoticeInfo& InInfo)
{
	// 넘치면 가장 오래된 줄(맨 위)부터 뺀다. 엔트리 목록은 붙인 순서를 유지한다
	while (FeedContainer->GetNumEntries() >= MaxFeedRows)
	{
		UUserWidget* OldestEntry = FeedContainer->GetAllEntries()[0];
		if (UDC1FeedNoticeRowWidget* OldestRow = Cast<UDC1FeedNoticeRowWidget>(OldestEntry))
		{
			OldestRow->StopNotice();
		}

		FeedContainer->RemoveEntry(OldestEntry);
	}

	UDC1FeedNoticeRowWidget* Row = FeedContainer->CreateEntry<UDC1FeedNoticeRowWidget>();
	if (nullptr == Row)
	{
		return;
	}

	// 엔트리 위젯은 재사용되므로 묶기를 매번 덮어쓴다
	Row->OnRowFinished.BindUObject(this, &ThisClass::RemoveFeedRow);
	Row->ShowNotice(InInfo);
}

void UDC1NotificationWidget::RemoveFeedRow(UDC1FeedNoticeRowWidget* InRow)
{
	if (nullptr == InRow)
	{
		return;
	}

	InRow->StopNotice();
	FeedContainer->RemoveEntry(InRow);
}
