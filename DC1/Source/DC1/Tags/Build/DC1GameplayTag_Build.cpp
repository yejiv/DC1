#include "Tags/Build/DC1GameplayTag_Build.h"

namespace DC1BuildTags
{
#pragma region Snap Point
	UE_DEFINE_GAMEPLAY_TAG(Build_Snap_Side, "Build.Snap.Side");

	UE_DEFINE_GAMEPLAY_TAG(Build_Snap_Anchor_Wall, "Build.Snap.Anchor.Wall");
	UE_DEFINE_GAMEPLAY_TAG(Build_Snap_Anchor_Roof, "Build.Snap.Anchor.Roof");

	UE_DEFINE_GAMEPLAY_TAG(Build_Snap_Attach_Wall, "Build.Snap.Attach.Wall");
	UE_DEFINE_GAMEPLAY_TAG(Build_Snap_Attach_Roof, "Build.Snap.Attach.Roof");
#pragma endregion

#pragma region CATEGORY_BUILD
	UE_DEFINE_GAMEPLAY_TAG(Build, "Build");

	#pragma region CRAFTING_CRAFTANDREAPIR
		UE_DEFINE_GAMEPLAY_TAG(Build_Crafting_CraftAndRepair_WorkBench00, "Build.Crafting.CraftAndRepair.WorkBench00");
	#pragma endregion
	
	#pragma region CRAFTING_SPHERE
		UE_DEFINE_GAMEPLAY_TAG(Build_Crafting_Sphere_SphereBench00, "Build.Crafting.Sphere.SphereBench00");
	#pragma endregion
	
	#pragma region CRAFTING_WEAPON
		UE_DEFINE_GAMEPLAY_TAG(Build_Crafting_Weapon_WeaponBench00, "Build.Crafting.Weapon.WeaponBench00");
	#pragma endregion
	
	#pragma region PAL_PALMANAGEMENT
		UE_DEFINE_GAMEPLAY_TAG(Build_Pal_PalManagement_PalBox, "Build.Pal.PalManagement.PalBox");
	#pragma endregion
	
	#pragma region FOOD_COOK
	#pragma endregion
	
	#pragma region FOOD_FARM
		UE_DEFINE_GAMEPLAY_TAG(Build_Food_Farm_Fruit, "Build.Food.Farm.Fruit");
	#pragma endregion
	
	#pragma region BUILDING
		UE_DEFINE_GAMEPLAY_TAG(Build_Building_Wood_Floor, "Build.Building.Wood.Floor");
		UE_DEFINE_GAMEPLAY_TAG(Build_Building_Wood_Wall, "Build.Building.Wood.Wall");
		UE_DEFINE_GAMEPLAY_TAG(Build_Building_Wood_Roof, "Build.Building.Wood.Roof");
	#pragma endregion


#pragma endregion


#pragma region CATEGORY_CRAFTING
	UE_DEFINE_GAMEPLAY_TAG(Crafting_Weapon, "Crafting.Weapon");
	UE_DEFINE_GAMEPLAY_TAG(Crafting_Armor_Body, "Crafting.Armor.Body");
	UE_DEFINE_GAMEPLAY_TAG(Crafting_Sphere, "Crafting.Sphere");
#pragma endregion
}