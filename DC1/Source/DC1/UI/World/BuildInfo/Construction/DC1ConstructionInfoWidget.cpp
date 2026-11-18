#include "UI/World/BuildInfo/Construction/DC1ConstructionInfoWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/WidgetSwitcher.h"
#include "Animation/WidgetAnimation.h"

void UDC1ConstructionInfoWidget::OnAnimFinished(const UWidgetAnimation* InAnim)
{
	if (false == ::IsValid(InAnim) || InAnim != PalWorkerExpandAnim)
	{
		return;
	}

	if (false == HasPalWorker)
	{
		SetPalWorkerAreaVisible(false);
	}
}

void UDC1ConstructionInfoWidget::OnPalWorkerCountChanged(int32 InPrevCount, int32 InNewCount, bool InIsInitial)
{
	UE_LOG(LogTemp, Warning, TEXT("Constructed : %d"), IsConstructed());

	FDC1AnimPlayInfo Forward;
	FDC1AnimPlayInfo Reverse;
	Reverse.PlayMode = EUMGSequencePlayMode::Reverse;

	HasPalWorker = InNewCount > 0;
	if (true == HasPalWorker)
	{
		SetPalWorkerAreaVisible(true);
		if (true == InIsInitial)
		{
			SnapAnimToEdge(PalWorkerExpandAnim, Forward);
		}
		else if (InPrevCount <= 0)
		{
			PlayAnim(PalWorkerExpandAnim, Forward);
		}
	}
	else
	{
		if (true == InIsInitial)
		{
			SnapAnimToEdge(PalWorkerExpandAnim, Reverse);
			SetPalWorkerAreaVisible(false);
		}
		else
		{
			PlayAnim(PalWorkerExpandAnim, Reverse);
		}
	}
}

void UDC1ConstructionInfoWidget::Subscribe(ADC1InteractBuildActor* InBuildActor)
{
	// SetInfoState에서 유효성 검사
	WorkProgressChangedHandle = InBuildActor->OnWorkProgressChanged.AddUObject(this, &ThisClass::RefreshValues);
}

void UDC1ConstructionInfoWidget::Unsubscribe(ADC1InteractBuildActor* InBuildActor)
{
	if (true == ::IsValid(InBuildActor))
	{
		InBuildActor->OnWorkProgressChanged.Remove(WorkProgressChangedHandle);
	}

	WorkProgressChangedHandle.Reset();
}

void UDC1ConstructionInfoWidget::RefreshValues()
{
	ADC1InteractBuildActor* Actor = BuildActor.Get();
	if (false == ::IsValid(Actor))
	{
		return;
	}

	if (true == ::IsValid(BuildProgressBar))
	{
		BuildProgressBar->SetPercent(Actor->GetBuildProgress());
	}

	bool HasWorker = Actor->GetCurrentWorkSpeed() > 0.f;

	SetActivePanel(HasWorker ? EDC1ConstructionInfoPanel::Working : EDC1ConstructionInfoPanel::NoWorker);

	if (true == HasWorker && true == ::IsValid(RemainingTimeText))
	{
		FText TimeText = FormatRemainingTime(FMath::RoundToInt(Actor->GetRemainingWork() / Actor->GetCurrentWorkSpeed()));
		RemainingTimeText->SetText(TimeText);
	}
	else if (false == HasWorker && true == ::IsValid(RemainingWorkText))
	{
		FString WorkString = FString::Printf(TEXT("%.1f"), Actor->GetRemainingWork());
		RemainingWorkText->SetText(FText::FromString(WorkString));
	}
}

FText UDC1ConstructionInfoWidget::FormatRemainingTime(int32 InSeconds)
{
	int32 Hours = 0;
	int32 Minutes = 0;
	int32 Seconds = 0;

	if (InSeconds > 0)
	{
		Hours = InSeconds / 3600;
		Minutes = (InSeconds % 3600) / 60;
		Seconds = InSeconds % 60;
	}

	FString TimeString = FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Seconds);

	return FText::FromString(TimeString);
}

void UDC1ConstructionInfoWidget::SetActivePanel(EDC1ConstructionInfoPanel InPanel)
{
	int32* Index = PanelToSwitchIndex.Find(InPanel);
	if (nullptr == Index || nullptr == WorkerStateSwitcher->GetWidgetAtIndex(*Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1ConstructionInfoWidget :: Don't Find PanelToSwitchIndex - [%s]"), *UEnum::GetValueAsString(InPanel));
		return;
	}

	WorkerStateSwitcher->SetActiveWidgetIndex(*Index);
}

void UDC1ConstructionInfoWidget::SetPalWorkerAreaVisible(bool InIsVisible)
{
	PalWorkerArea->SetVisibility(InIsVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}
