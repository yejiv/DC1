// Fill out your copyright notice in the Description page of Project Settings.

#include "Tags/Pal/DC1GameplayTag_Pal.h"

namespace DC1PalTags
{
#pragma region State
	UE_DEFINE_GAMEPLAY_TAG(Pal_State_Idle, "Pal.State.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Pal_State_Chase, "Pal.State.Chase");
	UE_DEFINE_GAMEPLAY_TAG(Pal_State_Attack, "Pal.State.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Pal_State_Dead, "Pal.State.Dead");
#pragma endregion

#pragma region Ability
	UE_DEFINE_GAMEPLAY_TAG(Pal_Ability_Attack_Melee, "Pal.Ability.Attack.Melee");
#pragma endregion
}
