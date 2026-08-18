// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 *
 */
namespace DC1PalTags
{
#pragma region Species
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_Alpaca);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_Anubis);
#pragma endregion

#pragma region Ability
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_Idle);
#pragma endregion

#pragma region Event
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Pal_Attack_Hit);
#pragma endregion
}
