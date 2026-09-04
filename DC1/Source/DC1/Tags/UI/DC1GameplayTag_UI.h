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

#pragma region HUD
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_HUD);
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

#pragma region CHEST_LOOT
	// 상자 개방 시 플레이어 인벤토리 + 상자 인벤토리를 함께 보여주는 화면
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Chest_Loot);
#pragma endregion
}
