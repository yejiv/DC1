#pragma once

#include "NativeGameplayTags.h"

namespace DC1Labels
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Label_Preload);
}

namespace DC1AssetTags
{
#pragma region Ability
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_Ability_Player);
#pragma endregion

#pragma region Anim
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_Anim_Player);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_AnimInstance_Weapon_None);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_AnimInstance_Weapon_Melee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_AnimInstance_Weapon_Bow);
#pragma endregion

#pragma region Input
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_InputData);
#pragma endregion

#pragma region UI
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_UI_WidgetData);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_UI_LoadingData);

	// UDC1UIResourceData. 하드 참조 DA라 Preload 라벨에도 넣어야 한다
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_UI_Resource_Common);
#pragma endregion

#pragma region DataTable
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_DataTableData);
#pragma endregion

#pragma region Pal
	// 종 데이터는 여기에 등록하지 않는다. 레지스트리만 프리로드하고 종 데이터는 소프트 레퍼런스로 따라간다.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_Pal_Registry);

	// PalData.AbilityDataTag가 가리킬 DC1AbilityData의 에셋 태그.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_Ability_Pal);
#pragma endregion

#pragma region Appearance
	// 외형 선택지 레지스트리. 메시/아이콘은 소프트 레퍼런스로 따라간다
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_Appearance_Registry);
#pragma endregion
}


namespace DC1DataTableTags
{
#pragma region UI
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(DataTable_UI_LoadingTip);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(DataTable_UI_TechTree);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(DataTable_UI_Build);
#pragma endregion

#pragma region Item
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(DataTable_Item);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(DataTable_EquipItem);
#pragma endregion
}