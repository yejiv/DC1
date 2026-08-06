// Fill out your copyright notice in the Description page of Project Settings.

#include "Tags/Pal/DC1GameplayTag_Pal.h"

namespace DC1PalTags
{
#pragma region State
	UE_DEFINE_GAMEPLAY_TAG(Pal_State_Idle, "Pal.State.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Pal_State_Follow, "Pal.State.Follow");
	UE_DEFINE_GAMEPLAY_TAG(Pal_State_Combat, "Pal.State.Combat");
	UE_DEFINE_GAMEPLAY_TAG(Pal_State_Flee, "Pal.State.Flee");
	UE_DEFINE_GAMEPLAY_TAG(Pal_State_Gather, "Pal.State.Gather");
	UE_DEFINE_GAMEPLAY_TAG(Pal_State_Ride, "Pal.State.Ride");
#pragma endregion

#pragma region Command
	UE_DEFINE_GAMEPLAY_TAG(Pal_Command_Follow, "Pal.Command.Follow");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Command_Attack, "Pal.Command.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Command_Gather, "Pal.Command.Gather");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Command_Wait, "Pal.Command.Wait");
#pragma endregion

#pragma region Ability
	UE_DEFINE_GAMEPLAY_TAG(Pal_Ability_Attack_Melee, "Pal.Ability.Attack.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Ability_Attack_Ranged, "Pal.Ability.Attack.Ranged");
#pragma endregion
}
