// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 *
 */
namespace DC1PalTags
{
#pragma region State
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_State_Idle);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_State_Follow);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_State_Combat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_State_Flee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_State_Gather);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_State_Ride);
#pragma endregion

#pragma region Command
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Command_Follow);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Command_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Command_Gather);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Command_Wait);
#pragma endregion

#pragma region Ability
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Ability_Attack_Melee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Ability_Attack_Ranged);
#pragma endregion
}
