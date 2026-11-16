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
		UE_DEFINE_GAMEPLAY_TAG(Build_Crafting_CraftAndRepair_WorkBench01, "Build.Crafting.CraftAndRepair.WorkBench01");
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
		UE_DEFINE_GAMEPLAY_TAG(Build_Crafting_MillingAndGrinding_Pulverizer, "Build.Crafting.MillingAndGrinding.Pulverizer");
	#pragma endregion

	#pragma region PAL_PALMANAGEMENT
		UE_DEFINE_GAMEPLAY_TAG(Build_Pal_PalManagement_PalBox, "Build.Pal.PalManagement.PalBox");
	#pragma endregion

	#pragma region STORAGE_BOX
		UE_DEFINE_GAMEPLAY_TAG(Build_Storage_Box_Chest, "Build.Storage.Box.Chest");
		UE_DEFINE_GAMEPLAY_TAG(Build_Storage_Box_MetalChest, "Build.Storage.Box.MetalChest");
	#pragma endregion
	
	#pragma region FOOD_COOK
		UE_DEFINE_GAMEPLAY_TAG(Build_Food_Cook_CampFire, "Build.Food.Cook.CampFire");
		UE_DEFINE_GAMEPLAY_TAG(Build_Food_Cook_CookingPot, "Build.Food.Cook.CookingPot");
	#pragma endregion
	
	#pragma region FOOD_FARM
		UE_DEFINE_GAMEPLAY_TAG(Build_Food_Farm_Fruit, "Build.Food.Farm.Fruit");
		UE_DEFINE_GAMEPLAY_TAG(Build_Food_Farm_Wheat, "Build.Food.Farm.Wheat");
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
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Weapon_Woodclub, "Crafting.Weapon.Woodclub");							
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Weapon_WoodBow, "Crafting.Weapon.WoodBow");							
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Weapon_StoneAxe, "Crafting.Weapon.StoneAxe");								
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Weapon_StonePickaxe, "Crafting.Weapon.StonePickaxe");					

		UE_DEFINE_GAMEPLAY_TAG(Crafting_Weapon_WoodBat, "Crafting.Weapon.WoodBat");
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Weapon_MetalSword, "Crafting.Weapon.MetalSword");
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Weapon_MetalBow, "Crafting.Weapon.MetalBow");

		UE_DEFINE_GAMEPLAY_TAG(Crafting_Item_Consume_Arrow, "Crafting.Item.Consume.Arrow");
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Item_Material_WoodPlank, "Crafting.Item.Material.WoodPlank");
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Item_Material_Nail, "Crafting.Item.Material.Nail");
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Item_Material_Cloth, "Crafting.Item.Material.Cloth");
		
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Armor_Body_ClothClothes, "Crafting.Armor.Body.ClothClothes");
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Armor_Body_LeatherArmor, "Crafting.Armor.Body.LeatherArmor");
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Armor_Body_MetalArmor, "Crafting.Armor.Body.MetalArmor");

		UE_DEFINE_GAMEPLAY_TAG(Crafting_Armor_Head_LeatherArmor, "Crafting.Armor.Head.LeatherArmor");
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Armor_Head_MetalArmor, "Crafting.Armor.Head.MetalArmor");

		UE_DEFINE_GAMEPLAY_TAG(Crafting_Sphere_PalSphere, "Crafting.Sphere.PalSphere");
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Sphere_MegaSphere, "Crafting.Sphere.MegaSphere");

		UE_DEFINE_GAMEPLAY_TAG(Crafting_Glider_Normal, "Crafting.Glider.Normal");
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Glider_Mega, "Crafting.Glider.Mega");

		UE_DEFINE_GAMEPLAY_TAG(Crafting_Shield_Normal, "Crafting.Shield.Normal");
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Shield_Mega, "Crafting.Shield.Mega");

		UE_DEFINE_GAMEPLAY_TAG(Crafting_Valuable_SmallPouch, "Crafting.Valuable.SmallPouch");
		UE_DEFINE_GAMEPLAY_TAG(Crafting_Valuable_SamllFoodbag, "Crafting.Valuable.SamllFoodbag");
#pragma endregion
}

namespace DC1InteractTags
{
	UE_DEFINE_GAMEPLAY_TAG(Interact_Work, "Interact.Work");
	UE_DEFINE_GAMEPLAY_TAG(Interact_Revive, "Interact.Revive");
}