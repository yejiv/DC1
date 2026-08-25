// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/Build/DC1BuildWidget.h"

#include "UI/Tab/Build/DC1BuildRowWidget.h"

#include "Component/DC1TechTreeComponent.h"

#include "Data/DataTable/DC1DataTableTypes.h"

#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

void UDC1BuildWidget::InitBuild(const TArray<FDC1BuildRow>& InBuildRows, const UDC1TechTreeComponent* InTechComp)
{
	if (true == ::IsValid(BuildTextLabel) && false == BuildText.IsEmpty())
	{
		BuildTextLabel->SetText(BuildText);
	}

	if (false == ::IsValid(BuildScrollBox) || false == ::IsValid(RowWidgetClass) || false == ::IsValid(InTechComp))
	{
		return;
	}

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
		BuildRows.Add(RowInfo.RowTag, RowWidget);
	}

	const TMap<FGameplayTag, const FDC1TechRow>& TechRowsByTag = InTechComp->GetTechRowsByTag();

	for (const FDC1BuildRow& BuildRow : InBuildRows)
	{
		const FGameplayTag& BuildTag = BuildRow.BuildTag;
		const FGameplayTag& RowTag = BuildTag.RequestDirectParent();

		const FDC1TechRow* FoundTech = TechRowsByTag.Find(BuildTag);
		if (nullptr == FoundTech)
		{
			UE_LOG(LogTemp, Warning, TEXT("UDC1BuildWidget :: Cant Find TechRow by %s"), *BuildTag.ToString());
			continue;
		}

		TObjectPtr<UDC1BuildRowWidget>* FoundRow = BuildRows.Find(RowTag);
		if (nullptr == FoundRow)
		{
			UE_LOG(LogTemp, Warning, TEXT("UDC1BuildWidget :: Cant Find TechRow by %s"), *BuildTag.ToString());
			continue;
		}
		
		(*FoundRow)->AddBuildSlot(BuildRow, *FoundTech);

		if (true == ::IsValid(InTechComp) && true == InTechComp->HasUnlockedTag(BuildTag))
		{
			(*FoundRow)->VisibleSlot(BuildTag);
		}
	}
}

bool UDC1BuildWidget::VisibleSlot(const FGameplayTag& InRowTag, const FGameplayTag& InSlotTag)
{
	TObjectPtr<UDC1BuildRowWidget>* Found = BuildRows.Find(InRowTag);
	if (nullptr == Found)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildWidget :: Cant Find TechRow by %s"), *InRowTag.ToString());
		return false;
	}

	return (*Found)->VisibleSlot(InSlotTag);
}
