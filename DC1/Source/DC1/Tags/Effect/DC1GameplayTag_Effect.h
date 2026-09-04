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
#pragma endregion
}
