// Fill out your copyright notice in the Description page of Project Settings.

#include "Tags/Pal/DC1GameplayTag_Pal.h"

namespace DC1PalTags
{
#pragma region Species
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_Alpaca, "Pal.Species.Alpaca");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_Anubis, "Pal.Species.Anubis");
#pragma endregion

#pragma region Ability
	UE_DEFINE_GAMEPLAY_TAG(Ability_Pal_Idle, "Ability.Pal.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Pal_Hit, "Ability.Pal.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Pal_Tossin, "Ability.Pal.Tossin");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Pal_FarSkill, "Ability.Pal.FarSkill");
#pragma endregion

#pragma region State
	UE_DEFINE_GAMEPLAY_TAG(State_Pal_Attacking, "State.Pal.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Pal_Idling, "State.Pal.Idling");
	UE_DEFINE_GAMEPLAY_TAG(State_Pal_HitReacting, "State.Pal.HitReacting");
#pragma endregion

#pragma region Anim
	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_Hit, "Anim.Pal.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_Tossin, "Anim.Pal.Tossin");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_FarSkill, "Anim.Pal.FarSkill");

	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_Idle_LookAround, "Anim.Pal.Idle.LookAround");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_Idle_Eat, "Anim.Pal.Idle.Eat");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_Idle_Rest, "Anim.Pal.Idle.Rest");
#pragma endregion

#pragma region Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Pal_Attack_Hit, "Event.Pal.Attack.Hit");
#pragma endregion
}
