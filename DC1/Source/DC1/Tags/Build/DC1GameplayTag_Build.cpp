#include "Tags/Build/DC1GameplayTag_Build.h"

namespace DC1BuildTags
{
#pragma region Link Point
	UE_DEFINE_GAMEPLAY_TAG(Build_Link_Surface, "Build.Link.Surface");

	UE_DEFINE_GAMEPLAY_TAG(Build_Snap_Side_Default, "Build.Snap.Side.Default");

	UE_DEFINE_GAMEPLAY_TAG(Build_Snap_Side_Floor, "Build.Snap.Side.Floor");
	UE_DEFINE_GAMEPLAY_TAG(Build_Snap_Side_Roof, "Build.Snap.Side.Roof");

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
	
	#pragma region CRAFTING_DEFOREST_AND_MINING
		UE_DEFINE_GAMEPLAY_TAG(Build_Crafting_DeforestAndMining_LoggingCamp, "Build.Crafting.DeforestAndMining.LoggingCamp");
		UE_DEFINE_GAMEPLAY_TAG(Build_Crafting_DeforestAndMining_StonePit, "Build.Crafting.DeforestAndMining.StonePit");
	#pragma endregion

	#pragma region CRAFTING_SMELTING
		UE_DEFINE_GAMEPLAY_TAG(Build_Crafting_Smelting_BlastFurnace, "Build.Crafting.Smelting.BlastFurnace");
	#pragma endregion

	#pragma region CRAFTING_MILLING_AND_GRINDING
		UE_DEFINE_GAMEPLAY_TAG(Build_Crafting_MillingAndGrinding_FlourMill, "Build.Crafting.MillingAndGrinding.FlourMill");
	#pragma endregion

	#pragma region PAL_PALMANAGEMENT
		UE_DEFINE_GAMEPLAY_TAG(Build_Pal_PalManagement_PalBox, "Build.Pal.PalManagement.PalBox");
	#pragma endregion

	#pragma region STORAGE_BOX
		UE_DEFINE_GAMEPLAY_TAG(Build_Storage_Box_Chest, "Build.Storage.Box.Chest");
	#pragma endregion
	
	#pragma region FOOD_COOK
		UE_DEFINE_GAMEPLAY_TAG(Build_Food_Cook_CampFire, "Build.Food.Cook.CampFire");							
	#pragma endregion
	
	#pragma region FOOD_FARM
		UE_DEFINE_GAMEPLAY_TAG(Build_Food_Farm_Fruit, "Build.Food.Farm.Fruit");
	#pragma endregion

	#pragma region INFRA_BEDDING
		UE_DEFINE_GAMEPLAY_TAG(Build_Infra_Bedding_LowGradeBed, "Build.Infra.Bedding.LowGradeBed");	
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

namespace DC1InteractTags
{
	UE_DEFINE_GAMEPLAY_TAG(Interact_Work, "Interact.Work");
}