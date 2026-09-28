#include "Define/DC1ItemHelper.h"
#include "Define/DC1Helper.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#pragma region Item Helper

const FDC1ItemRow* DC1ItemHelper::FindItemRow(FName InItemID)
{
	if (InItemID.IsNone())
	{
		return nullptr;
	}

	const UDataTable* ItemDataTable = DC1DataTableHelper::GetDataTable(DC1DataTableTags::DataTable_Item);
	if (false == ::IsValid(ItemDataTable))
	{
		return nullptr;
	}

	// 행 메모리를 FDC1ItemRow로 캐스팅하므로 행 구조체 타입을 먼저 확인
	const UScriptStruct* RowStruct = ItemDataTable->GetRowStruct();
	if (nullptr == RowStruct || false == RowStruct->IsChildOf(FDC1ItemRow::StaticStruct()))
	{
		return nullptr;
	}

	// 행 이름과 무관하게 ItemID 필드로 검색 (아이템 수가 많아지면 ItemID -> 행 캐시 도입 고려)
	for (const TPair<FName, uint8*>& RowPair : ItemDataTable->GetRowMap())
	{
		const FDC1ItemRow* ItemRow = reinterpret_cast<const FDC1ItemRow*>(RowPair.Value);
		if (nullptr != ItemRow && InItemID == ItemRow->ItemID)
		{
			return ItemRow;
		}
	}

	return nullptr;
}

#pragma endregion
