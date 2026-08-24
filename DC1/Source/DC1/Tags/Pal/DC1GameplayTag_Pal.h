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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_Idle);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_Hit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_Tossin);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_FarSkill);
#pragma endregion

#pragma region State
	// 어빌리티 진행 여부. StateTree 태스크가 이 태그로 종료를 판단한다
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Pal_Attacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Pal_Idling);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Pal_HitReacting);
#pragma endregion

#pragma region Anim
	// PalData.Montages의 키
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Hit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Tossin);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_FarSkill);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Idle_LookAround);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Idle_Eat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Idle_Rest);
#pragma endregion

#pragma region Event
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Pal_Attack_Hit);
#pragma endregion
}
