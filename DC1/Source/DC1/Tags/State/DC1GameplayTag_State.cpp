// Fill out your copyright notice in the Description page of Project Settings.

#include "Tags/State/DC1GameplayTag_State.h"

namespace DC1StateTags
{
#pragma region Player
	UE_DEFINE_GAMEPLAY_TAG(State_Aiming, "State.Aiming");
	UE_DEFINE_GAMEPLAY_TAG(State_ZoomIn, "State.ZoomIn");

	UE_DEFINE_GAMEPLAY_TAG(State_Dodging, "State.Dodging");
	UE_DEFINE_GAMEPLAY_TAG(State_Sprinting, "State.Sprinting");
	UE_DEFINE_GAMEPLAY_TAG(State_Climbing, "State.Climbing");
	UE_DEFINE_GAMEPLAY_TAG(State_Climbing_Moving, "State.Climbing.Moving");

	UE_DEFINE_GAMEPLAY_TAG(State_Weapon, "State.Weapon");
	UE_DEFINE_GAMEPLAY_TAG(State_Weapon_None, "State.Weapon.None");
	UE_DEFINE_GAMEPLAY_TAG(State_Weapon_Melee, "State.Weapon.Melee");
	UE_DEFINE_GAMEPLAY_TAG(State_Weapon_Bow, "State.Weapon.Bow");

	UE_DEFINE_GAMEPLAY_TAG(State_Build_Placement, "State.Build.Placement");

	UE_DEFINE_GAMEPLAY_TAG(State_Build_Working, "State.Build.Working");
#pragma endregion

#pragma region Common
	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Invincible, "State.Invincible");

	UE_DEFINE_GAMEPLAY_TAG(State_Stamina_RegenBlocked, "State.Stamina.RegenBlocked");
#pragma endregion
}