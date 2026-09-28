#pragma once

#include "CoreMinimal.h"

struct FDC1ItemRow;

#pragma region Item Helper

/**
 * 아이템 DataTable(DataTable_Item) 조회 헬퍼
 * 아이템은 행 이름(RowName)이 아니라 FDC1ItemRow::ItemID 필드로 식별한다 - 행 이름은 자유롭게 지정 가능
 */
namespace DC1ItemHelper
{
	/**
	 * ItemID 필드가 일치하는 행 조회 (없으면 nullptr)
	 * 같은 ItemID가 여러 행에 있으면 첫 번째 행을 반환
	 */
	const FDC1ItemRow* FindItemRow(FName InItemID);
}

#pragma endregion
