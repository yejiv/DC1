// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "NativeGameplayTags.h"

namespace DC1ItemTags
{
#pragma region Tool
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Tool_Hammer);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Tool_DeforestAxe);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Tool_MiningPickaxe);
#pragma endregion

#pragma region Type
	// FDC1ItemRow::CategoryTag - 작업대는 상위 태그로 제작 가능 분류를 지정
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Weapon_Melee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Weapon_Bow);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Weapon_Crossbow);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Weapon_Gun);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Weapon_Throw);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Weapon_FishingRod);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Sphere);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_SphereModule);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Armor_Body);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Armor_Head);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Shield);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Glider);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Accessory);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Consume_Ammo);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Consume_Medicine);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Consume_Bait);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Consume_Other);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Food_Ingredient);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Food_Cooked);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Material_Raw);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Material_Processed);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Essential);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Type_Schematic);
#pragma endregion

#pragma region Valuable
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Valuable);
#pragma endregion

#pragma region Money
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Money);
#pragma endregion
}