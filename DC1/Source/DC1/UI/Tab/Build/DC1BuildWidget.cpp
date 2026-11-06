// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/Build/DC1BuildWidget.h"

#include "UI/Tab/Build/DC1BuildRowWidget.h"

#include "Data/DataTable/DC1DataTableTypes.h"

#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

#include "ViewModel/Build/DC1BuildViewModel.h"

void UDC1BuildWidget::InitBuild(UDC1BuildViewModel* InViewModel)
{
	if (true == ::IsValid(BuildTextLabel) && false == BuildText.IsEmpty())
	{
		BuildTextLabel->SetText(BuildText);
	}

	if (false == ::IsValid(BuildScrollBox) || false == ::IsValid(RowWidgetClass) || false == ::IsValid(InViewModel))
	{
		return;
	}

	if (true == IsInit)
	{
		return;
	}

	IsInit = true;

	for (const FDC1BuildRowInfo& RowInfo : RowInfos)
	{
		UDC1BuildRowWidget* RowWidget = CreateWidget<UDC1BuildRowWidget>(this, RowWidgetClass);
		if (false == ::IsValid(RowWidget))
		{
			continue;
		}

		// Init Row - Setting Tag, Name, Visible
		RowWidget->InitRow(RowInfo.RowTag, RowInfo.RowName);

		BuildScrollBox->AddChild(RowWidget);
		BuildRowsByTag.Add(RowInfo.RowTag, RowWidget);
	}

	const TArray<FDC1BuildRow>& BuildRows = InViewModel->GetBuildRows();
	for (const FDC1BuildRow& BuildRow : BuildRows)
	{
		const FGameplayTag& BuildTag = BuildRow.BuildTag;
		const FGameplayTag& RowTag = BuildTag.RequestDirectParent();

		UDC1BuildRowWidget* RowWidget = BuildRowsByTag.FindRef(RowTag);
		if (false == ::IsValid(RowWidget))
		{
			UE_LOG(LogTemp, Warning, TEXT("UDC1BuildWidget :: Can't Find RowWidget by %s"), *BuildTag.ToString());
			continue;
		}

		RowWidget->AddBuildSlot(BuildRow, InViewModel);

		if (true == InViewModel->IsBuildUnlocked(BuildTag))
		{
			RowWidget->VisibleSlot(BuildTag);
		}
	}
}

bool UDC1BuildWidget::VisibleSlot(const FGameplayTag& InRowTag, const FGameplayTag& InSlotTag)
{
	UDC1BuildRowWidget* RowWidget = BuildRowsByTag.FindRef(InRowTag);
	if (false == ::IsValid(RowWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildWidget :: Can't Find TechRow by %s"), *InRowTag.ToString());
		return false;
	}

	return RowWidget->VisibleSlot(InSlotTag);
}

void UDC1BuildWidget::RefreshCostState(const ADC1PlayerState* InPlayerState)
{
	for (const TPair<FGameplayTag, TObjectPtr<UDC1BuildRowWidget>>& Pair : BuildRowsByTag)
	{
		if (false == ::IsValid(Pair.Value))
		{
			continue;
		}

		Pair.Value->RefreshCostState(InPlayerState);
	}
}
