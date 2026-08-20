// Fill out your copyright notice in the Description page of Project Settings.

#include "Tags/Player/DC1GameplayTag_Player.h"

namespace DC1PlayerTags
{
#pragma region Ability
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Attack, "Ability.Player.Attack");
#pragma endregion

#pragma region Event
	UE_DEFINE_GAMEPLAY_TAG(Event_Player_Hit, "Event.Player.Hit");
#pragma endregion
}
