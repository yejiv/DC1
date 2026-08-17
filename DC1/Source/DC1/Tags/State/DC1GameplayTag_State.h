// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace DC1StateTags
{
#pragma region Player
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Weapon);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Weapon_None);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Weapon_Melee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Weapon_Bow);
#pragma endregion

#pragma region Common
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
#pragma endregion
}