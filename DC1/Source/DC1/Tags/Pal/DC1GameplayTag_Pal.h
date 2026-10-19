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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_Boar);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_CaptainPenguin);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_ChickenPal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_CowPal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_CuteFox);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_Deer);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_DreamDemon);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_ElecPanda);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_ElecPomeranian);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_Garm);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_GhostBlackCat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_GrassMammoth);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_HadesBird);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_HawkBird);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_Kitsunebi);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_LeafMomonga);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_Monkey);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_Penguin);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_PinkCat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Species_SheepBall);
#pragma endregion
#pragma region Skill
	// 돈질
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_RecklessCharge);	// 저돌맹진       
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_FluffyTackle);		// 폭신 태클      
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_RolyPoly);			// 구르기         
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_BullRush);			// 불 러시

	// 투사체
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_AirCannon);		// 에어 캐넌      
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_PowerShot);		// 파워 샷        
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_SeedMachineGun);	// 시드 머신건    
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_WindCutter);		// 윈드 커터      
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_BogBlast);			// 진흙탄         
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_StoneBlast);		// 스톤 블래스트  
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_SparkBlast);		// 스파크 블래스트 
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_ElectricBall);		// 일렉트릭 볼    
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_AquaGun);			// 아쿠아 건      
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_IgnisBlast);		// 이그니스 블래스트 
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_IceMissile);		// 아이스 미사일  
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_DragonCannon);		// 드래곤 캐넌    
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_DarkBall);			// 다크 볼        
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Pal_Skill_DarkShot);			// 다크 샷        
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

	// 작업 종류( 구 Pal.Work.* )는 Tags/Work/DC1GameplayTag_Work ( 펠 · 플레이어 공용 )

#pragma region Ability
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_Idle);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_Hit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_Tossin);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_FarSkill);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_Encount);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Pal_Work);

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

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Work_Building);		// 건설
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Work_Kindling);		// 불피우기
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Work_Watering);		// 관개
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Work_Planting);		// 파종
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Work_Electricity);	// 발전
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Work_Handiwork);	// 제작
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Work_Gathering);	// 채집
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Work_Lumbering);	// 벌목
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Work_Mining);		// 채굴
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Work_Medicine);		// 제약
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Work_Cooling);		// 냉각
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Work_Transporting);	// 운반
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Work_Farming);		// 목장
	
#pragma endregion

#pragma region Event
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Pal_Attack_Hit);
#pragma endregion

#pragma region Ride
	// 탑승 중에만 켜는 IMC. DA_InputData 의 IMC 키
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(IMC_Ride);

	// DA_InputData 의 IA 키
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ride_Move);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ride_Jump);
#pragma endregion
}
