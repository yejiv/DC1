// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 *
 */
namespace DC1PalTags
{
#pragma region Species
	// 특정 종을 이름으로 참조X
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_Alpaca);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_Anubis);
#pragma endregion

#pragma region Ability
	// DA에서 이 태그를 GA에 붙이면 ASC의 ActivateAbilityByTag로 찾아진다.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_Attack);
#pragma endregion

#pragma region State
	//이 태그의 유무로 StateTree 태스크가 종료를 판단한다.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_State_Attacking);
#pragma endregion

#pragma region Event
	// 공격 몽타주의 AnimNotify가 이 이벤트를 쏘면 어빌리티가 받아서 판정 트레이스를 돌린다.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Pal_Attack_Hit);
#pragma endregion

#pragma region SetByCaller
	// GE_PalDamage의 크기를 런타임에 넘기는 키. AttackPower가 여기로 들어간다.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Damage);
#pragma endregion
}
