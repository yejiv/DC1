#pragma once

#include "NativeGameplayTags.h"

namespace DC1UITags
{
#pragma region GAMEINSTANCE
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Fade);
#pragma endregion

#pragma region LOGO
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Logo_Title);
#pragma endregion

#pragma region HUD
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_HUD);
#pragma endregion

#pragma region MAIN_TAB

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Tab);

	// UI_Main_Tab에서 Tab 구별용 Tag
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Tab_Inventory);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Tab_HasPal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Tab_TechTree);
#pragma endregion

#pragma region MAIN_BUILD
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Build);
#pragma endregion
}


namespace DC1TechTags
{
#pragma region CATEGORY_BUILD
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build);


#pragma region CRAFTING_CRAFTANDREAPIR
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Crafting_CraftAndRepair_WorkBench00);
#pragma endregion


#pragma region CRAFTING_SPHERE
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Crafting_Sphere_SphereBench00);
#pragma endregion


#pragma region CRAFTING_WEAPON
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Crafting_Weapon_WeaponBench00);
#pragma endregion


#pragma region PAL_PALMANAGEMENT
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Pal_PalManagement_PalBox);
#pragma endregion


#pragma region FOOD_COOK
#pragma endregion


#pragma region FOOD_FARM
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Food_Farm_Fruit);
#pragma endregion


#pragma endregion


#pragma region CATEGORY_CRAFTING
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Crafting_Weapon);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Crafting_Armor_Body);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Crafting_Sphere);
#pragma endregion
}