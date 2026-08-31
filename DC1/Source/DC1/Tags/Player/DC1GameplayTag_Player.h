// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 *
 */
namespace DC1PlayerTags
{
#pragma region Ability
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_PalSphere);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_BuildWork);
#pragma endregion

#pragma region Event
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Hit);
#pragma endregion
}
