// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/BuildInfo/DC1BuildInfoWidget.h"

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
		
		IsSubscribed = true;
		
		RefreshValues();
	}

	OnInfoStateChanged(InPrevState, InNewState);
}

void UDC1BuildInfoWidget::UnsubscribeInternal()
{
	Unsubscribe(BuildActor.Get());
	IsSubscribed = false;
}
