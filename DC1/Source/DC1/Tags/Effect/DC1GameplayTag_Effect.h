// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace DC1EffectTags
{
#pragma region SetByCaller
	// 피해량을 담는 칸의 이름. 피해를 주는 모든 GE가 공유
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Damage);

	// 스태미나 소모량을 담는 칸의 이름. 스태미나를 쓰는 모든 GE가 공유
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_StaminaCost);

	// 팰 패시브가 스탯에 곱할 배율을 담는 칸. UDC1GameplayEffect_PalPassive 가 읽음.
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_PalPassive_Attack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_PalPassive_Defense);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_PalPassive_WorkSpeed);
#pragma endregion
}
