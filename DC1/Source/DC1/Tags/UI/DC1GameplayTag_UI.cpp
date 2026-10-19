#include "Tags/UI/DC1GameplayTag_UI.h"

namespace DC1UITags
{
#pragma region GAMEINSTANCE
	UE_DEFINE_GAMEPLAY_TAG(UI_Fade, "UI.Fade");
#pragma endregion

#pragma region LOGO
	UE_DEFINE_GAMEPLAY_TAG(UI_Logo_Title, "UI.Logo.Title");
#pragma endregion

#pragma region HUD
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_HUD, "UI.Main.HUD");
#pragma endregion

#pragma region MAIN TAB
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_Tab, "UI.Main.Tab");
	
	// UI_Main_Tab의 Tab 구별용 Tag
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_Tab_Inventory,	"UI.Main.Tab.Inventory");
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_Tab_HasPal,		"UI.Main.Tab.HasPal");
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_Tab_TechTree,	"UI.Main.Tab.TechTree");
#pragma endregion

#pragma region MAIN_BUILD
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_Build, "UI.Main.Build");
#pragma endregion

#pragma region MAIN_PALBOX
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_PalBox, "UI.Main.PalBox");
#pragma endregion

#pragma region MAIN_TELEPORT
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_Teleport, "UI.Main.Teleport");
#pragma endregion

#pragma region CHEST_LOOT
	UE_DEFINE_GAMEPLAY_TAG(UI_Chest_Loot, "UI.Chest.Loot");
#pragma endregion

#pragma region CUSTOMIZE
	UE_DEFINE_GAMEPLAY_TAG(UI_Customize, "UI.Customize");
#pragma endregion
}
