// Fill out your copyright notice in the Description page of Project Settings.


#include "Tags/Input/DC1GameplayTag_Input.h"

namespace DC1IMCTags
{
	UE_DEFINE_GAMEPLAY_TAG(IMC_Default, "IMC.Default");
	UE_DEFINE_GAMEPLAY_TAG(IMC_UI, "IMC.UI");
	UE_DEFINE_GAMEPLAY_TAG(IMC_Build, "IMC.Build");
	UE_DEFINE_GAMEPLAY_TAG(IMC_Downed, "IMC.Downed");
	UE_DEFINE_GAMEPLAY_TAG(IMC_Interact_F, "IMC.Interact.F");
	UE_DEFINE_GAMEPLAY_TAG(IMC_Interact_V, "IMC.Interact.V");
	UE_DEFINE_GAMEPLAY_TAG(IMC_Interact_C, "IMC.Interact.C");
}

namespace DC1InputTags
{
#pragma region Default
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_Move, "Input.Default.Move");
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_Turn, "Input.Default.Turn");
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_Sprint, "Input.Default.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_Toggle_Move, "Input.Default.Toggle.Move");
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_Jump, "Input.Default.Jump");
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_Crouch, "Input.Default.Crouch");
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_Dodge, "Input.Default.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_ClimbRelease, "Input.Default.ClimbRelease");
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_Attack, "Input.Default.Attack");

	UE_DEFINE_GAMEPLAY_TAG(Input_Default_ZoomIn, "Input.Default.ZoomIn");
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_LockOn, "Input.Default.LockOn");
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_Throw, "Input.Default.Throw");
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_CallPal, "Input.Default.CallPal");
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_PalSlotPrev, "Input.Default.PalSlotPrev");
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_PalSlotNext, "Input.Default.PalSlotNext");
	UE_DEFINE_GAMEPLAY_TAG(Input_Default_Ride, "Input.Default.Ride");

	UE_DEFINE_GAMEPLAY_TAG(Input_Default_ChangeWeapon, "Input.Default.ChangeWeapon");
#pragma endregion

#pragma region UI
	UE_DEFINE_GAMEPLAY_TAG(Input_UI_Inventory, "Input.UI.Inventory");
	UE_DEFINE_GAMEPLAY_TAG(Input_UI_HasPal, "Input.UI.HasPal");
	UE_DEFINE_GAMEPLAY_TAG(Input_UI_TechTree, "Input.UI.TechTree");

	UE_DEFINE_GAMEPLAY_TAG(Input_UI_Build, "Input.UI.Build");

	UE_DEFINE_GAMEPLAY_TAG(Input_UI_PalBox, "Input.UI.PalBox");
#pragma endregion

#pragma region Downed
	UE_DEFINE_GAMEPLAY_TAG(Input_Downed_GiveUp, "Input.Downed.GiveUp");
#pragma endregion
}

namespace DC1InputBuildTags
{
	UE_DEFINE_GAMEPLAY_TAG(Input_Build_Confirm, "Input.Build.Confirm");
	UE_DEFINE_GAMEPLAY_TAG(Input_Build_Cancel, "Input.Build.Cancel");
	UE_DEFINE_GAMEPLAY_TAG(Input_Build_Rotate, "Input.Build.Rotate");
	UE_DEFINE_GAMEPLAY_TAG(Input_Build_Snap, "Input.Build.Snap");
}

namespace DC1InputInteractTags
{
	UE_DEFINE_GAMEPLAY_TAG(Input_Interact_F, "Input.Interact.F");
	UE_DEFINE_GAMEPLAY_TAG(Input_Interact_V, "Input.Interact.V");
	UE_DEFINE_GAMEPLAY_TAG(Input_Interact_C, "Input.Interact.C");
}