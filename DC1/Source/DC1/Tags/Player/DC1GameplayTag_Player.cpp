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

	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Work, "Ability.Player.Work");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Downed, "Ability.Player.Downed");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Revive, "Ability.Player.Revive");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_CallPal, "Ability.Player.CallPal");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Eat, "Ability.Player.Eat");

#pragma endregion

#pragma region Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Player_Hit, "Event.Player.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Event_Player_Fired, "Event.Player.Fired");
	UE_DEFINE_GAMEPLAY_TAG(Event_Player_Damaged, "Event.Player.Damaged");
	UE_DEFINE_GAMEPLAY_TAG(Event_Player_CallPal, "Event.Player.CallPal");
#pragma endregion

#pragma region Exp
	UE_DEFINE_GAMEPLAY_TAG(Event_Exp, "Event.Exp");
	UE_DEFINE_GAMEPLAY_TAG(Event_Exp_Player, "Event.Exp.Player");
	UE_DEFINE_GAMEPLAY_TAG(Event_Exp_Player_Work, "Event.Exp.Player.Work");
	UE_DEFINE_GAMEPLAY_TAG(Event_Exp_Player_Build, "Event.Exp.Player.Build");
	UE_DEFINE_GAMEPLAY_TAG(Event_Exp_Player_Gather, "Event.Exp.Player.Gather");
	UE_DEFINE_GAMEPLAY_TAG(Event_Exp_Shared, "Event.Exp.Shared");
	UE_DEFINE_GAMEPLAY_TAG(Event_Exp_Shared_Kill, "Event.Exp.Shared.Kill");
	UE_DEFINE_GAMEPLAY_TAG(Event_Exp_Shared_Capture, "Event.Exp.Shared.Capture");
	UE_DEFINE_GAMEPLAY_TAG(Event_Exp_Pal, "Event.Exp.Pal");
	UE_DEFINE_GAMEPLAY_TAG(Event_Player_Eat, "Event.Player.Eat");
#pragma endregion

#pragma region Cue
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Player_Hit, "GameplayCue.Player.Hit");
#pragma endregion
}
