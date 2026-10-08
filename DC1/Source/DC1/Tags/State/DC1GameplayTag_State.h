// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace DC1StateTags
{
#pragma region Player
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Aiming);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_ZoomIn);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dodging);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Sprinting);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Climbing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Climbing_Moving);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Weapon);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Weapon_None);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Weapon_Melee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Weapon_Bow);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Build_Placement);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Working);
#pragma endregion

#pragma region Common
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Invincible);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Stamina_RegenBlocked);
#pragma endregion
}