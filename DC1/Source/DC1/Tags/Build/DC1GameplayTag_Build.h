#pragma once

#include "NativeGameplayTags.h"

namespace DC1BuildTags
{
#pragma region Link Point
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Link_Surface);			// 바닥 or 지붕 표면과 연결

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Snap_Side_Default);	// 단순 건축물 ( 옆면만 스냅 )

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Snap_Side_Floor);		// 바닥끼리 스냅
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Snap_Side_Roof);		// 지붕끼리 스냅

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Snap_Anchor_Wall);		// 벽이 붙을 위치
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Snap_Anchor_Roof);		// 지붕이 붙을 위치

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Snap_Attach_Wall);		// 벽이 붙는 위치
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Snap_Attach_Roof);		// 지붕이 붙는 위치
#pragma endregion

#pragma region CATEGORY_BUILD
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build);

	#pragma region CRAFTING_CRAFTANDREAPIR
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Crafting_CraftAndRepair_WorkBench00);			// 원시적인 작업대
	#pragma endregion
	
	
	#pragma region CRAFTING_SPHERE
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Crafting_Sphere_SphereBench00);				// 팰 스피어 제작대
	#pragma endregion
	
	#pragma region CRAFTING_WEAPON
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Crafting_Weapon_WeaponBench00);				// 무기 제작대
	#pragma endregion
	
	#pragma region CRAFTING_DEFOREST_AND_MINING
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Crafting_DeforestAndMining_LoggingCamp);		// 벌목장
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Crafting_DeforestAndMining_StonePit);			// 채굴장
	#pragma endregion
	
	#pragma region CRAFTING_SMELTING
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Crafting_Smelting_BlastFurnace);				// 원시적인 화로
	#pragma endregion
		
	#pragma region CRAFTING_MILLING_AND_GRINDING
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Crafting_MillingAndGrinding_FlourMill);		// 제분기
	#pragma endregion

	#pragma region PAL_PALMANAGEMENT
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Pal_PalManagement_PalBox);						// 팰박스
	#pragma endregion
	
	#pragma region STORAGE_BOX
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Storage_Box_Chest);							// 상자
	#pragma endregion

	#pragma region FOOD_COOK
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Food_Cook_CampFire);							// 모닥불
	#pragma endregion
	
	
	#pragma region FOOD_FARM
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Food_Farm_Fruit);								// 열매 농원
	#pragma endregion
	
	#pragma region INFRA_BEDDING
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Infra_Bedding_LowGradeBed);					// 하급 침대
	#pragma endregion

	#pragma region BUILDING_WOOD
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Building_Wood_Floor);							// 나무 바닥
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Building_Wood_Wall);							// 나무 벽
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Building_Wood_Roof);							// 나무 지붕
	#pragma endregion

#pragma endregion



#pragma region CATEGORY_CRAFTING
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Crafting_Weapon);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Crafting_Armor_Body);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Crafting_Sphere);
#pragma endregion
}

// 상호작용 옵션의 활동 종류 - 건축물은 "무엇을 할 수 있는지"만, 행위자가 자기 Ability로 변환 ( UDC1InteractComponent )
namespace DC1InteractTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interact_Work);		// 작업 ( 건설 · 생산 - 종류는 대상 GetWorkTypeTag )
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interact_Revive);	// 기절한 팀원 치료
}