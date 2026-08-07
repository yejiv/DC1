// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DA/DataTable/DC1DataTableData.h"
#include "Engine/DataTable.h"

UDataTable* UDC1DataTableData::GetDataTable(const FGameplayTag& TableTag) const
{
	TObjectPtr<UDataTable> const* Found = DataTables.Find(TableTag);
	if (nullptr == Found)
	{
		return nullptr;
	}

	return *Found;
}
