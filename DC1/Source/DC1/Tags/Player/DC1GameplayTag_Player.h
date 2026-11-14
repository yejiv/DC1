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

	// 작업 GA 하나 - 작업 종류는 대상에서 ( UDC1GA_Player_Work::WorkSets )
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Work);

	// 기절 ( HP 0 → 사망 전 단계 ). 서버가 활성화
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Downed);

	// 기절한 팀원 치료 ( F 홀드 )
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Revive);

	// 팰 소환/회수 ( E )
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_CallPal);

#pragma endregion

#pragma region Event
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Hit);

	// 원거리 발사 순간. 조준점 반동 같은 1회성 연출의 신호로 쓴다
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Fired);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_Damaged);

	// 팰 소환/회수 GA 활성화. 소유 클라가 보낸다 ( EventMagnitude = 선택 슬롯 )
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Player_CallPal);
#pragma endregion

#pragma region Exp
	// 경험치 지급 ( UDC1ExperienceSubsystem::Grant ). EventMagnitude = 양, Instigator = Earner
	// AbilityTriggers 로 쓰지 말 것 - 구독으로만 받는다
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Exp);

	// 플레이어만 ( 행동 )
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Exp_Player);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Exp_Player_Work);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Exp_Player_Build);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Exp_Player_Gather);		// 자원 채집

	// 플레이어 + 팰 ( 처치/포획 )
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Exp_Shared);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Exp_Shared_Kill);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Exp_Shared_Capture);

	// 팰만
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Exp_Pal);
#pragma endregion

#pragma region Cue
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Player_Hit);
#pragma endregion
}
