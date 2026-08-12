#pragma once

#include "NativeGameplayTags.h"

namespace DC1UITags
{
#pragma region GAMEINSTANCE
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Fade);
#pragma endregion

#pragma region LOGO
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Logo_Title);
#pragma endregion

#pragma region MAIN
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Tab);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Tab_Inventory);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Tab_HasPal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Tab_TechTree);
#pragma endregion
}


namespace DC1TechTags
{
#pragma region CATEGORY_BUILD
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Crafting_CraftAndRepair);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Crafting_Sphere);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Crafting_Weapon);
#pragma endregion

#pragma region CATEGORY_PAL
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_Pal_PalBox);
#pragma endregion
}