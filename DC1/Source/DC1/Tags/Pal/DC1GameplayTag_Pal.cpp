// Fill out your copyright notice in the Description page of Project Settings.

#include "Tags/Pal/DC1GameplayTag_Pal.h"

namespace DC1PalTags
{
#pragma region Species
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_Alpaca, "Pal.Species.Alpaca");
	UE_DEFINE_GAMEPLAY_TAG(Pal_Species_Anubis, "Pal.Species.Anubis");
#pragma endregion

#pragma region Ability
	UE_DEFINE_GAMEPLAY_TAG(Ability_Pal_Attack, "Ability.Pal.Attack");
#pragma endregion

#pragma region State
	UE_DEFINE_GAMEPLAY_TAG(Pal_State_Attacking, "Pal.State.Attacking");
#pragma endregion
}
