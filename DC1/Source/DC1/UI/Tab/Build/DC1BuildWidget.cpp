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

		TObjectPtr<UDC1BuildRowWidget>* FoundRow = BuildRowsByTag.Find(RowTag);
		if (nullptr == FoundRow)
		{
			UE_LOG(LogTemp, Warning, TEXT("UDC1BuildWidget :: Cant Find RowWidget by %s"), *BuildTag.ToString());
			continue;
		}

		(*FoundRow)->AddBuildSlot(BuildRow, InViewModel);

		if (true == InViewModel->IsBuildUnlocked(BuildTag))
		{
			(*FoundRow)->VisibleSlot(BuildTag);
		}
	}
}

bool UDC1BuildWidget::VisibleSlot(const FGameplayTag& InRowTag, const FGameplayTag& InSlotTag)
{
	TObjectPtr<UDC1BuildRowWidget>* Found = BuildRowsByTag.Find(InRowTag);
	if (nullptr == Found)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildWidget :: Cant Find TechRow by %s"), *InRowTag.ToString());
		return false;
	}

	return (*Found)->VisibleSlot(InSlotTag);
}
