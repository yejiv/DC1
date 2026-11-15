#include "Tags/Asset/DC1GameplayTag_Asset.h"

namespace DC1Labels
{
	UE_DEFINE_GAMEPLAY_TAG(Label_Preload, "Label.Preload");
}

namespace DC1AssetTags
{
#pragma region Ability
	UE_DEFINE_GAMEPLAY_TAG(Asset_Ability_Player, "Asset.Ability.Player");
#pragma endregion

#pragma region Anim
	UE_DEFINE_GAMEPLAY_TAG(Asset_Anim_Player, "Asset.Anim.Player");
	UE_DEFINE_GAMEPLAY_TAG(Asset_AnimInstance_Weapon_None, "Asset.AnimInstance.Weapon.None");
	UE_DEFINE_GAMEPLAY_TAG(Asset_AnimInstance_Weapon_Melee, "Asset.AnimInstance.Weapon.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Asset_AnimInstance_Weapon_Bow, "Asset.AnimInstance.Weapon.Bow");
#pragma endregion

#pragma region Input
	UE_DEFINE_GAMEPLAY_TAG(Asset_InputData, "Asset.InputData");
#pragma endregion

#pragma region UI
	UE_DEFINE_GAMEPLAY_TAG(Asset_UI_WidgetData, "Asset.UI.WidgetData");
	UE_DEFINE_GAMEPLAY_TAG(Asset_UI_LoadingData, "Asset.UI.LoadingData");
	UE_DEFINE_GAMEPLAY_TAG(Asset_UI_Resource_Common, "Asset.UI.Resource.Common");
#pragma endregion

#pragma region Sound
	UE_DEFINE_GAMEPLAY_TAG(Asset_SoundData, "Asset.SoundData");
#pragma endregion

#pragma region DataTable
	UE_DEFINE_GAMEPLAY_TAG(Asset_DataTableData, "Asset.DataTableData");
#pragma endregion

#pragma region Pal
	UE_DEFINE_GAMEPLAY_TAG(Asset_Pal_Registry, "Asset.Pal.Registry");
	UE_DEFINE_GAMEPLAY_TAG(Asset_Ability_Pal, "Asset.Ability.Pal");
#pragma endregion

#pragma region Appearance
	UE_DEFINE_GAMEPLAY_TAG(Asset_Appearance_Registry, "Asset.Appearance.Registry");
#pragma endregion

#pragma region Player
	UE_DEFINE_GAMEPLAY_TAG(Asset_Player_ExpData, "Asset.Player.ExpData");
#pragma endregion
}

namespace DC1DataTableTags
{
#pragma region UI
	UE_DEFINE_GAMEPLAY_TAG(DataTable_UI_LoadingTip, "DataTable.UI.LoadingTip");
	UE_DEFINE_GAMEPLAY_TAG(DataTable_UI_TechTree,	"DataTable.UI.TechTree");
	UE_DEFINE_GAMEPLAY_TAG(DataTable_UI_Build,		"DataTable.UI.Build");
#pragma endregion

#pragma region Item
	UE_DEFINE_GAMEPLAY_TAG(DataTable_Item, "DataTable.Item");
	UE_DEFINE_GAMEPLAY_TAG(DataTable_ValuableItem, "DataTable.ValuableItem");
	UE_DEFINE_GAMEPLAY_TAG(DataTable_EquipItem, "DataTable.EquipItem");
	UE_DEFINE_GAMEPLAY_TAG(DataTable_FoodItem, "DataTable.FoodItem");
	UE_DEFINE_GAMEPLAY_TAG(DataTable_ItemRecipe, "DataTable.ItemRecipe");
#pragma endregion
}