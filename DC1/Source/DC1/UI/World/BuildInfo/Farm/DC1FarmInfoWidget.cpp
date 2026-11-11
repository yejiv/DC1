// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/BuildInfo/Farm/DC1FarmInfoWidget.h"
#include "Actor/BuildActor/Interact/Production/Farm/DC1FarmBuildActor.h"

#include "Components/TextBlock.h"

void UDC1FarmInfoWidget::RefreshValues()
{
	Super::RefreshValues();

	ADC1FarmBuildActor* FarmActor = Cast<ADC1FarmBuildActor>(BuildActor.Get());
	if (false == ::IsValid(FarmActor) || false == ::IsValid(CurrentPhaseText) || false == ::IsValid(NextPhasePanel) || false == ::IsValid(NextPhaseText))
	{
		return;
	}

	const TArray<FDC1FarmPhaseInfo>& Phases = FarmActor->GetPhases();
	const int32 PhaseIndex = FarmActor->GetPhaseIndex();
	if (false == Phases.IsValidIndex(PhaseIndex))
	{
		return;
	}

	CurrentPhaseText->SetText(Phases[PhaseIndex].OptionText);

	const int32 NextIndex = PhaseIndex + 1;
	const bool HasNextPhase = Phases.IsValidIndex(NextIndex);

	NextPhasePanel->SetVisibility(HasNextPhase ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	if (true == HasNextPhase)
	{
		NextPhaseText->SetText(Phases[NextIndex].OptionText);
	}
}
