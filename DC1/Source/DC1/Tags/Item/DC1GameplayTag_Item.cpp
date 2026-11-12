// Fill out your copyright notice in the Description page of Project Settings.


#include "Tags/Item/DC1GameplayTag_Item.h"

namespace DC1ItemTags
{
#pragma region Tool
	UE_DEFINE_GAMEPLAY_TAG(Item_Tool_Hammer, "Item.Tool.Hammer");
	UE_DEFINE_GAMEPLAY_TAG(Item_Tool_DeforestAxe, "Item.Tool.DeforestAxe");
	UE_DEFINE_GAMEPLAY_TAG(Item_Tool_MiningPickaxe, "Item.Tool.MiningPickaxe");
	UE_DEFINE_GAMEPLAY_TAG(Item_Tool_WaterningPot, "Item.Tool.WaterningPot");
	UE_DEFINE_GAMEPLAY_TAG(Item_Tool_Pan, "Item.Tool.Pan");
#pragma endregion

#pragma region Type
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Weapon_Melee, "Item.Type.Weapon.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Weapon_Bow, "Item.Type.Weapon.Bow");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Weapon_Crossbow, "Item.Type.Weapon.Crossbow");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Weapon_Gun, "Item.Type.Weapon.Gun");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Weapon_Throw, "Item.Type.Weapon.Throw");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Weapon_FishingRod, "Item.Type.Weapon.FishingRod");

	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Sphere, "Item.Type.Sphere");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_SphereModule, "Item.Type.SphereModule");

	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Armor_Body, "Item.Type.Armor.Body");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Armor_Head, "Item.Type.Armor.Head");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Shield, "Item.Type.Shield");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Glider, "Item.Type.Glider");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Accessory, "Item.Type.Accessory");

	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Consume_Ammo, "Item.Type.Consume.Ammo");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Consume_Medicine, "Item.Type.Consume.Medicine");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Consume_Bait, "Item.Type.Consume.Bait");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Consume_Other, "Item.Type.Consume.Other");

	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Food_Ingredient, "Item.Type.Food.Ingredient");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Food_Cooked, "Item.Type.Food.Cooked");

	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Material_Raw, "Item.Type.Material.Raw");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Material_Processed, "Item.Type.Material.Processed");

	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Essential, "Item.Type.Essential");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Schematic, "Item.Type.Schematic");
#pragma endregion

#pragma region Valuable
	UE_DEFINE_GAMEPLAY_TAG(Item_Valuable, "Item.Valuable");
#pragma endregion

#pragma region Money
	UE_DEFINE_GAMEPLAY_TAG(Item_Money, "Item.Money");
#pragma endregion
}

