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
}

namespace DC1InputTags
{
#pragma region Default
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Move);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Turn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Sprint);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Toggle_Move);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Jump);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Attack);
#pragma endregion


#pragma region UI
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_UI_TechTree);
#pragma endregion
}