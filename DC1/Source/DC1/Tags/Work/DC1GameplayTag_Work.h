// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 * 작업 종류 - 건축물이 요구하는 작업 ( ADC1InteractBuildActor::GetWorkTypeTag )
 * 펠 · 플레이어 공용 어휘. 할 수 있는 작업은 각자 선언 ( 펠 = UDC1PalData::WorkSuitability, 플레이어 = UDC1GA_Player_Work::WorkSets )
 */
namespace DC1WorkTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Work_Kindling);			// 불 피우기
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Work_Watering);			// 관개
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Work_Planting);			// 파종
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Work_Electricity);		// 발전
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Work_Handiwork);			// 수작업
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Work_Gathering);			// 채집
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Work_Lumbering);			// 벌목
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Work_Mining);			// 채굴
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Work_Medicine);			// 제약
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Work_Cooling);			// 냉각
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Work_Transporting);		// 운반
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Work_Farming);			// 목장

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Work_Handiwork_Building);// 건설
}
