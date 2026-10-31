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

	// 알림 층 (포획 성공 등). HUD 처럼 오버레이로 항상 떠 있다
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Notification);
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

#pragma region MAIN_PALBOX
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_PalBox);
#pragma endregion

#pragma region MAIN_TELEPORT
	// 텔레포트 지점 목록 화면
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Main_Teleport);
#pragma endregion

#pragma region CHEST_LOOT
	// 상자 개방 시 플레이어 인벤토리 + 상자 인벤토리를 함께 보여주는 화면
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Chest_Loot);
#pragma endregion

#pragma region CUSTOMIZE
	// 타이틀(최초) / 게임 중 재커스터마이징 양쪽에서 공용으로 Push하는 화면
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Customize);
#pragma endregion
}
