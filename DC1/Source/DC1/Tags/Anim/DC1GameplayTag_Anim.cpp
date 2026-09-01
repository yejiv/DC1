#include "Tags/Anim/DC1GameplayTag_Anim.h"

namespace DC1AnimTags
{
#pragma region Player
	UE_DEFINE_GAMEPLAY_TAG(Anim_Weapon_None, "Anim.Weapon.None");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Weapon_Melee, "Anim.Weapon.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Weapon_Bow, "Anim.Weapon.Bow");

	UE_DEFINE_GAMEPLAY_TAG(Anim_Attack_None, "Anim.Attack.None");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Attack_Melee, "Anim.Attack.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Attack_Bow, "Anim.Attack.Bow");

	UE_DEFINE_GAMEPLAY_TAG(Anim_Action_Throw, "Anim.Action.Throw");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Action_Dodge_Roll, "Anim.Action.Dodge.Roll");
	UE_DEFINE_GAMEPLAY_TAG(Anim_Action_Dodge_Step, "Anim.Action.Dodge.Step");

	UE_DEFINE_GAMEPLAY_TAG(Anim_Build_Working, "Anim.Build.Working");
#pragma endregion
}
