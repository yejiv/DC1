#pragma once

#include "Engine/DataTable.h"

struct FGameplayTag;

namespace DC1DataTableHelper
{
	const UDataTable* GetDataTable(const FGameplayTag& InTableTag);

	template<typename RowType>
	bool GetAllRows(const FGameplayTag& InTableTag, TArray<RowType>& OutRows)
	{
		const UDataTable* Table = GetDataTable(InTableTag);
		if (false == ::IsValid(Table))
		{
			return false;
		}

		TArray<RowType*> AllRows;
		Table->GetAllRows<RowType>(TEXT("DC1DataTableHelper"), AllRows);
		
		OutRows.Reset();
		OutRows.Reserve(AllRows.Num());

		for (const RowType* Row : AllRows)
		{
			if (nullptr != Row)
			{
				OutRows.Add(*Row);
			}
		}

		return (false == OutRows.IsEmpty());
	}
}