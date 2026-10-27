#pragma once

#include "NativeGameplayTags.h"

namespace DC1AnimTags
{
#pragma region Player
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Weapon_None);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Weapon_Melee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Weapon_Bow);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Attack_None);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Attack_Melee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Attack_Bow);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Draw_None);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Draw_Melee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Draw_Bow);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Action_Throw);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Action_Dodge_Roll);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Action_Dodge_Step);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Action_ClimbAtTop);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Action_Death);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Action_Downed);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Action_Working_Build);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Action_Working_Deforest);
#pragma endregion
}
