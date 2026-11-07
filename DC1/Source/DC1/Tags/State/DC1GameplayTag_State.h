// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace DC1StateTags
{
#pragma region Player
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Aiming);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_ZoomIn);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dodging);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Sprinting);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Climbing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Climbing_Moving);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Gliding);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Weapon);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Weapon_None);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Weapon_Melee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Weapon_Bow);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Build_Placement);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Working);

	// 기절 ( HP 0 → 사망 전 단계, 이동만 가능 )
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Downed);

	// 팀원에게 치료받는 중 ( 기절 게이지 자연감소 정지 ). State.Downed 하위로 두지 않는다
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_BeingRevived);
#pragma endregion

#pragma region Common
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Invincible);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Stamina_RegenBlocked);
#pragma endregion
}