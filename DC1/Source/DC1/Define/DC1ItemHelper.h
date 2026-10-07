#pragma once

#include "CoreMinimal.h"

struct FDC1ItemRow;
struct FDC1EquipItemRow;

#pragma region Item Helper

// 아이템 DataTable 조회 헬퍼 (행 이름이 아니라 ItemID 필드로 식별)
namespace DC1ItemHelper
{
	// 아이템 공통 데이터 조회 함수 (없으면 nullptr)
	const FDC1ItemRow* FindItemRow(FName InItemID);

	// 장비 데이터 조회 함수 (없으면 nullptr = 장비 불가)
	const FDC1EquipItemRow* FindEquipItemRow(FName InItemID);
}

#pragma endregion
