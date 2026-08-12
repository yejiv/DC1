#include "Tags/UI/DC1GameplayTag_UI.h"

namespace DC1UITags
{
#pragma region GAMEINSTANCE
	UE_DEFINE_GAMEPLAY_TAG(UI_Fade, "UI.Fade");
#pragma endregion

#pragma region LOGO
	UE_DEFINE_GAMEPLAY_TAG(UI_Logo_Title, "UI.Logo.Title");
#pragma endregion

#pragma region MAIN
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_Tab, "UI.Main.Tab");
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_Tab_Inventory,	"UI.Main.Tab.Inventory");
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_Tab_HasPal,		"UI.Main.Tab.HasPal");
	UE_DEFINE_GAMEPLAY_TAG(UI_Main_Tab_TechTree,	"UI.Main.Tab.TechTree");
#pragma endregion
}

namespace DC1TechTags
{
#pragma region CATEGORY_BUILD
	UE_DEFINE_GAMEPLAY_TAG(Build_Crafting_CraftAndRepair, "Build.Crafting.CraftAndRepair");
	UE_DEFINE_GAMEPLAY_TAG(Build_Crafting_Sphere, "Build.Crafting.Sphere");
	UE_DEFINE_GAMEPLAY_TAG(Build_Crafting_Weapon, "Build.Crafting.Weapon");
	
	UE_DEFINE_GAMEPLAY_TAG(Build_Pal_PalBox, "Build.Pal.PalBox");
#pragma endregion

#pragma region CATEGORY_CRAFTING
	UE_DEFINE_GAMEPLAY_TAG(Crafting_Weapon, "Crafting.Weapon");
	UE_DEFINE_GAMEPLAY_TAG(Crafting_Armor_Body, "Crafting.Armor.Body");
	UE_DEFINE_GAMEPLAY_TAG(Crafting_Sphere, "Crafting.Sphere");
#pragma endregion
}