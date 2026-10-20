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

#pragma region Consumption
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_PalSphere);
#pragma endregion

#pragma region Valuable
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Valuable);
#pragma endregion

#pragma region Money
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Money);
#pragma endregion
}