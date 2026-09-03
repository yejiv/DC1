#include "Tags/UI/DC1GameplayTag_UI.h"

namespace DC1UITags
{
#pragma region GAMEINSTANCE
	UE_DEFINE_GAMEPLAY_TAG(UI_Fade, "UI.Fade");
#pragma endregion

#pragma region LOGO
	UE_DEFINE_GAMEPLAY_TAG(UI_Logo_Title, "UI.Logo.Title");
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

	// UI_Main_Build의 Tab 구별용 Tag
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_Build_Crafting, "UI.Main.Build.Crafting");
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_Build_Pal, "UI.Main.Build.Pal");
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_Build_Storage, "UI.Main.Storage");
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_Build_Food, "UI.Main.Food");
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_Build_Infrastructure, "UI.Main.Infrastructure");
#pragma endregion
}