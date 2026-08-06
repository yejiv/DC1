// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 *
 */
namespace DC1PalTags
{
#pragma region State
	// 상태의 주인은 StateTree다. 진입/이탈 시 ASC에 게시만 하고, AI가 이 태그를 읽어 분기하지 않는다.
	// 읽는 쪽은 GameplayEffect / GameplayAbility의 차단·조건 태그.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_State_Idle);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_State_Chase);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_State_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_State_Dead);
#pragma endregion

#pragma region Ability
	// 부모 Pal.Ability.Attack은 태그 계층에 자동 생성되므로 따로 선언하지 않는다.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Ability_Attack_Melee);
#pragma endregion
}
