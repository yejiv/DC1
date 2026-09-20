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

	// 개체마다 다르게 붙는 특성. 배율과 랭크는 UDC1PalRegistryData 의 PassiveChart 가 갖는다.
	// 여기엔 이름만 있고 수치는 없다 - 수치를 바꿀 때 코드를 다시 빌드하지 않도록
#pragma region Passive
	// 공격력만
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_Ferocity);		// 사나움
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_Courageous);		// 용감
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_Coward);			// 겁쟁이
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_Pacifist);		// 무사주의자

	// 방어력만
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_ThickHide);		// 단단한 피부
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_Frail);			// 약한 맷집
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_BrittleBones);	// 부실한 뼈

	// 작업 속도만
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_Artisan);		// 장인 기질
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_Diligence);		// 성실함
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_Clumsiness);		// 서투름
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_SlackerHabit);	// 농땡이치는 버릇

	// 공격력 <-> 방어력
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_DemonGod);		// 귀신
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_Sadist);			// 사디스트
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_Masochist);		// 마조히스트
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_Delinquent);		// 불량배

	// 공격력 <-> 작업 속도
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_Musclehead);		// 운동 바보
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_Ferocious);		// 난폭함
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_WorkSlave);		// 일 노예

	// 방어력 <-> 작업 속도
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Passive_Conceited);		// 자만심
#pragma endregion

#pragma region Ability
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_Idle);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_Hit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_Tossin);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_FarSkill);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_Encount);
#pragma endregion

#pragma region State
	// 어빌리티 진행 여부. StateTree 태스크가 이 태그로 종료를 판단한다
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Pal_Attacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Pal_Idling);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Pal_HitReacting);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Pal_Encounting);
#pragma endregion

#pragma region Anim
	// PalData.Montages의 키
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Hit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Tossin);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_FarSkill);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Encount);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Idle_LookAround);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Idle_Eat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Idle_Rest);
#pragma endregion

#pragma region Event
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Pal_Attack_Hit);
#pragma endregion
}
