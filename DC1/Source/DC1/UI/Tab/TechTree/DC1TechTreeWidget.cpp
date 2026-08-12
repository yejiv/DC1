// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/TechTree/DC1TechTreeWidget.h"

#include "UI/Tab/TechTree/DC1TechTreeRowWidget.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/UI/DC1UIData.h"
#include "Data/DA/DataTable/DC1DataTableData.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/UI/DC1GameplayTag_UI.h"

#include "Components/ScrollBox.h"

void UDC1TechTreeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BuildTechTree();
}

void UDC1TechTreeWidget::BuildTechTree()
{
	if (false == ::IsValid(TechTreeScrollBox))
	{
		return;
	}

	const UDC1DataTableData* DataTable = UDC1AssetManager::GetAssetByTag<UDC1DataTableData>(DC1AssetTags::Asset_DataTableData);

	if (false == ::IsValid(DataTable))
	{
		return;
	}

	if (false == ::IsValid(RowWidgetClass))
	{
		return;
	}

	if (false == ::IsValid(CachedDataTable))
	{
		CachedDataTable = DataTable->GetDataTable(DC1DataTableTags::DataTable_UI_TechTree);
		if (false == ::IsValid(CachedDataTable))
		{
			return;
		}
	}


	TArray<FDC1TechRow*> AllRows;
	CachedDataTable->GetAllRows<FDC1TechRow>(TEXT("BuildTechTree"), AllRows);
	if (true == AllRows.IsEmpty())
	{
		return;
	}

	TSet<int32> AddRowLevels;
	for (const FDC1TechRow* Row : AllRows)
	{
		int32 Level = Row->RequiredLevel;
		AddRowLevels.Add(Level);
	}

	TArray<int32> Levels = AddRowLevels.Array();
	Levels.Sort();

	for (int32 AddLevel : Levels)
	{
		UDC1TechTreeRowWidget* RowWidget = CreateWidget<UDC1TechTreeRowWidget>(GetWorld(), RowWidgetClass);
		if (false == ::IsValid(RowWidget))
		{
			continue;
		}

		RowWidget->InitTier(AddLevel);
		TechTreeScrollBox->AddChild(RowWidget);
	}
}
