#include "Tags/Anim/DC1GameplayTag_Anim.h"

namespace DC1AnimTags
{
#pragma region Player
	UE_DEFINE_GAMEPLAY_TAG(Anim_Weapon_None, "Anim.Weapon.None");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Weapon_Melee, "Anim.Weapon.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Weapon_Bow, "Anim.Weapon.Bow");
#pragma endregion

#pragma region Pal
	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_Attack, "Anim.Pal.Attack");

	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_Idle_LookAround, "Anim.Pal.Idle.LookAround");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_Idle_Eat, "Anim.Pal.Idle.Eat");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Pal_Idle_Rest, "Anim.Pal.Idle.Rest");
#pragma endregion
}
