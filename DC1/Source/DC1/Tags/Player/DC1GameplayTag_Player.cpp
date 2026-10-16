// Fill out your copyright notice in the Description page of Project Settings.

#include "Tags/Player/DC1GameplayTag_Player.h"

namespace DC1PlayerTags
{
#pragma region Ability
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Attack, "Ability.Player.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_PalSphere, "Ability.Player.PalSphere");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Dodge, "Ability.Player.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Sprint, "Ability.Player.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_ClimbAtTop, "Ability.Player.ClimbAtTop");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_BuildWork, "Ability.Player.BuildWork");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_ProductionWork, "Ability.Player.ProductionWork");
#pragma endregion

#pragma region Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Player_Hit, "Event.Player.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Event_Player_Fired, "Event.Player.Fired");
	UE_DEFINE_GAMEPLAY_TAG(Event_Player_Damaged, "Event.Player.Damaged");
#pragma endregion

#pragma region Cue
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Player_Hit, "GameplayCue.Player.Hit");
#pragma endregion
}
