#pragma once

#include "CoreMinimal.h"

struct FDC1ItemRow;
struct FDC1ValuableItemRow;
struct FDC1EquipItemRow;

#pragma region Item Helper

// 아이템 DataTable 조회 헬퍼 (행 이름이 아니라 ItemID 필드로 식별)
namespace DC1ItemHelper
{
	// 아이템 공통 데이터 조회 함수 (일반 → 귀중품 테이블 순, 없으면 nullptr)
	const FDC1ItemRow* FindItemRow(FName InItemID);

	// 귀중품 데이터 조회 함수 (없으면 nullptr = 귀중품 아님)
	const FDC1ValuableItemRow* FindValuableItemRow(FName InItemID);

	bool IsValuableItem(FName InItemID);

	// 장비 데이터 조회 함수 (없으면 nullptr = 장비 불가)
	const FDC1EquipItemRow* FindEquipItemRow(FName InItemID);
}

#pragma endregion
