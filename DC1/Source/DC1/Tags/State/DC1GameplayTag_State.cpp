// Fill out your copyright notice in the Description page of Project Settings.

#include "Tags/State/DC1GameplayTag_State.h"

namespace DC1StateTags
{
#pragma region Player
	UE_DEFINE_GAMEPLAY_TAG(State_Weapon, "State.Weapon");
	UE_DEFINE_GAMEPLAY_TAG(State_Weapon_None, "State.Weapon.None");
	UE_DEFINE_GAMEPLAY_TAG(State_Weapon_Melee, "State.Weapon.Melee");
	UE_DEFINE_GAMEPLAY_TAG(State_Weapon_Bow, "State.Weapon.Bow");
#pragma endregion
}