// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/BuildInfo/DC1BuildInfoWidget.h"
#include "UI/World/BuildInfo/DC1PalWorkerEntryWidget.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Components/DynamicEntryBox.h"

void UDC1BuildInfoWidget::NativeDestruct()
{
	UnbindBuildActor();

	Super::NativeDestruct();
}

void UDC1BuildInfoWidget::BindBuildActor(ADC1InteractBuildActor* InBuildActor)
{
	if (false == ::IsValid(InBuildActor) || InBuildActor == BuildActor.Get())
	{
		return;
	}

	UnbindBuildActor();

	BuildActor = InBuildActor;
}

void UDC1BuildInfoWidget::UnbindBuildActor()
{
	UnsubscribeInternal();

	BuildActor = nullptr;

	CurrentInfoState = EDC1BuildInfoState::Hidden;
}

void UDC1BuildInfoWidget::SetInfoState(EDC1BuildInfoState InPrevState, EDC1BuildInfoState InNewState)
{
	CurrentInfoState = InNewState;

	if (CurrentInfoState == EDC1BuildInfoState::Hidden)
	{
		UnsubscribeInternal();
	}
	else if (false == IsSubscribed && true == BuildActor.IsValid())
	{
		Subscribe(BuildActor.Get());
		PalWorkersChangedHandle = BuildActor->OnPalWorkersChanged.AddUObject(this, &ThisClass::HandlePalWorkersChanged);

		IsSubscribed = true;
		
		RefreshValues();
		RefreshPalWorkers(true);
	}

	OnInfoStateChanged(InPrevState, InNewState);
}

void UDC1BuildInfoWidget::UnsubscribeInternal()
{
	Unsubscribe(BuildActor.Get());
	if (true == BuildActor.IsValid())
	{
		BuildActor.Get()->OnPalWorkersChanged.Remove(PalWorkersChangedHandle);
	}

	PalWorkersChangedHandle.Reset();
	ClearPalEntries();
	DisplayedPalCount = 0;

	IsSubscribed = false;
}

void UDC1BuildInfoWidget::RefreshPalWorkers(bool InIsInitial)
{
	ClearPalEntries();
	if (false == BuildActor.IsValid())
	{
		return;
	}

	int32 Count = 0;
	for (const TWeakObjectPtr<ADC1PalCharacter>& Pal : BuildActor.Get()->GetPalWorkers())
	{
		if (false == Pal.IsValid())
		{
			continue;
		}

		if (Count >= MaxVisiblePalEntries)
		{
			break;
		}

		UDC1PalWorkerEntryWidget* Entry = PalListBox->CreateEntry<UDC1PalWorkerEntryWidget>();
		if (false == ::IsValid(Entry))
		{
			UE_LOG(LogTemp, Warning, TEXT("UDC1BuildInfoWidget:: Failed Create UDC1PalWorkerEntryWidget"));
			break;
		}

		Entry->SetPal(Pal.Get());
		++Count;
	}

	if (true == InIsInitial || Count != DisplayedPalCount)
	{
		int32 Prev = DisplayedPalCount;
		DisplayedPalCount = Count;

		OnPalWorkerCountChanged(Prev, DisplayedPalCount, InIsInitial);
	}
}

void UDC1BuildInfoWidget::HandlePalWorkersChanged()
{
	RefreshPalWorkers(false);
}

void UDC1BuildInfoWidget::ClearPalEntries()
{
	if (false == ::IsValid(PalListBox))
	{
		return;
	}

	PalListBox->Reset<UDC1PalWorkerEntryWidget>([](UDC1PalWorkerEntryWidget& InEntry) {
		InEntry.ClearPal();
		});
}
