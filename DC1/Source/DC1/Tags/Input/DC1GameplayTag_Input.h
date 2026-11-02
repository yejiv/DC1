// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 * 
 */
namespace DC1IMCTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(IMC_Default);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(IMC_UI);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(IMC_Build);
	// 기절 중 기본 IMC ( 이동 / 시점 / 포기 )
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(IMC_Downed);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(IMC_Interact_F);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(IMC_Interact_V);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(IMC_Interact_C);
}

namespace DC1InputTags
{
#pragma region Default
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Move);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Turn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Sprint);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Toggle_Move);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Jump);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Crouch);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Dodge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_ClimbRelease);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Attack);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_ZoomIn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_LockOn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Throw);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_CallPal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_PalSlotPrev);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_PalSlotNext);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Ride);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_ChangeWeapon);
#pragma endregion


#pragma region UI
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_UI_Inventory);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_UI_HasPal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_UI_TechTree);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_UI_Build);
#pragma endregion

#pragma region Downed
	// 기절 중 포기 ( 즉시 사망 )
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Downed_GiveUp);
#pragma endregion
}

namespace DC1InputBuildTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Build_Confirm);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Build_Cancel);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Build_Rotate);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Build_Snap);
}

namespace DC1InputInteractTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Interact_F);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Interact_V);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Interact_C);
}