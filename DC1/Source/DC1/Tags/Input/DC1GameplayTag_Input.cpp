// Fill out your copyright notice in the Description page of Project Settings.


#include "Tags/Input/DC1GameplayTag_Input.h"

namespace DC1IMCTags
{
	UE_DEFINE_GAMEPLAY_TAG(IMC_Default, "IMC.Default");
}

namespace DC1InputTags
{
#pragma region Default
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_Move, "Input.Default.Move");
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_Turn, "Input.Default.Turn");
#pragma endregion
}