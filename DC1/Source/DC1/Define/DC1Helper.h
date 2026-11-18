#pragma once

#include "Engine/DataTable.h"

struct FGameplayTag;

#pragma region DataTable Helper

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

#pragma endregion

#pragma region Sound Helper

namespace DC1SoundHelper
{
	// 변형 사운드 여러 개 중 하나를 고른다 ( SoundCue 없이 랜덤 재생 ). 비었으면 nullptr
	template<typename SoundType>
	SoundType* PickRandom(const TArray<TObjectPtr<SoundType>>& InSounds)
	{
		return (true == InSounds.IsEmpty()) ? nullptr : InSounds[FMath::RandHelper(InSounds.Num())].Get();
	}
}

#pragma endregion