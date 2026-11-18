#include "Define/DC1ItemHelper.h"
#include "Define/DC1Helper.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/Item/DC1GameplayTag_Item.h"

#pragma region Item Helper

namespace
{
	// 전체 행을 돌며 ItemID 필드로 검색 (아이템이 많아지면 캐시 고려)
	template <typename TRow>
	const TRow* FindRowByItemID(const FGameplayTag& InDataTableTag, FName InItemID)
	{
		if (InItemID.IsNone())
		{
			return nullptr;
		}

		const UDataTable* DataTable = DC1DataTableHelper::GetDataTable(InDataTableTag);
		if (false == ::IsValid(DataTable))
		{
			return nullptr;
		}

		// 행 메모리를 TRow로 캐스팅하므로 행 구조체 타입을 먼저 확인
		const UScriptStruct* RowStruct = DataTable->GetRowStruct();
		if (nullptr == RowStruct || false == RowStruct->IsChildOf(TRow::StaticStruct()))
		{
			return nullptr;
		}

		for (const TPair<FName, uint8*>& RowPair : DataTable->GetRowMap())
		{
			const TRow* Row = reinterpret_cast<const TRow*>(RowPair.Value);
			if (nullptr != Row && InItemID == Row->ItemID)
			{
				return Row;
			}
		}

		return nullptr;
	}
}

const FDC1ItemRow* DC1ItemHelper::FindItemRow(FName InItemID)
{
	if (const FDC1ItemRow* ItemRow = FindRowByItemID<FDC1ItemRow>(DC1DataTableTags::DataTable_Item, InItemID))
	{
		return ItemRow;
	}

	return FindValuableItemRow(InItemID);
}

const FDC1ValuableItemRow* DC1ItemHelper::FindValuableItemRow(FName InItemID)
{
	return FindRowByItemID<FDC1ValuableItemRow>(DC1DataTableTags::DataTable_ValuableItem, InItemID);
}

bool DC1ItemHelper::IsValuableItem(FName InItemID)
{
	return nullptr != FindValuableItemRow(InItemID);
}

const FDC1EquipItemRow* DC1ItemHelper::FindEquipItemRow(FName InItemID)
{
	return FindRowByItemID<FDC1EquipItemRow>(DC1DataTableTags::DataTable_EquipItem, InItemID);
}

const FDC1FoodItemRow* DC1ItemHelper::FindFoodItemRow(FName InItemID)
{
	// 아이템 타입 확인
	const FDC1ItemRow* ItemRow = FindItemRow(InItemID);
	if (nullptr == ItemRow || false == ItemRow->CategoryTag.MatchesTag(DC1ItemTags::Item_Type_Food))
	{
		return nullptr;
	}

	return FindRowByItemID<FDC1FoodItemRow>(DC1DataTableTags::DataTable_FoodItem, InItemID);
}

#pragma endregion

#pragma region Recipe Helper

bool DC1RecipeHelper::FindRecipeRow(FName InRecipeID, FDC1ItemRecipeRow& OutRecipe)
{
	TArray<FDC1ItemRecipeRow> RecipeRows;
	if (false == DC1DataTableHelper::GetAllRows(DC1DataTableTags::DataTable_ItemRecipe, RecipeRows))
	{
		UE_LOG(LogTemp, Warning, TEXT("DC1RecipeHelper :: Cant Find RecipeRows from DataTable_ItemRecipe"));
		return false;
	}

		for (FDC1ItemRecipeRow& Row : RecipeRows)
	{
		if (true == Row.RecipeID.IsEqual(InRecipeID))
		{
			OutRecipe = Row;
			return true;
		}
	}

	return false;
}

#pragma endregion