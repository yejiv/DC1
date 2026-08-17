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
#pragma endregion

#pragma region Pal
	// PalData.Montages의 키
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Attack);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Idle_LookAround);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Idle_Eat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Pal_Idle_Rest);
#pragma endregion
}
