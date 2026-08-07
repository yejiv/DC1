// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 * 
 */
namespace DC1IMCTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(IMC_Default);
}

namespace DC1InputTags
{
#pragma region Default
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Move);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Turn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Sprint);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Toggle_Move);
#pragma endregion
}