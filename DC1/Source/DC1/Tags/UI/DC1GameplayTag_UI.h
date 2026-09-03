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

#pragma region MAIN_TAB

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Tab);

	// UI_Main_Tab에서 Tab 구별용 Tag
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Tab_Inventory);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Tab_HasPal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Tab_TechTree);
#pragma endregion

#pragma region MAIN_BUILD
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Build);
#pragma endregion
}
