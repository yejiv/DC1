// Fill out your copyright notice in the Description page of Project Settings.

#include "Tags/Effect/DC1GameplayTag_Effect.h"

namespace DC1EffectTags
{
#pragma region SetByCaller
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Damage, "SetByCaller.Damage");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_StaminaCost, "SetByCaller.StaminaCost");

	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_PalPassive_Attack, "SetByCaller.PalPassive.Attack");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_PalPassive_Defense, "SetByCaller.PalPassive.Defense");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_PalPassive_WorkSpeed, "SetByCaller.PalPassive.WorkSpeed");

	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Equip_Defense, "SetByCaller.Equip.Defense");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Equip_Shield, "SetByCaller.Equip.Shield");
	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Equip_Health, "SetByCaller.Equip.Health");
#pragma endregion
}
