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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Dodge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Sprint);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_ClimbAtTop);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_BuildWork);
#pragma endregion

#pragma region Event
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Hit);

	// 원거리 발사 순간. 조준점 반동 같은 1회성 연출의 신호로 쓴다
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Fired);
#pragma endregion
}
