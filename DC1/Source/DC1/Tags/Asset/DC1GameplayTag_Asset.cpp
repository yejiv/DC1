#include "Tags/Asset/DC1GameplayTag_Asset.h"

namespace DC1Labels
{
	UE_DEFINE_GAMEPLAY_TAG(Label_Preload, "Label.Preload");
}

namespace DC1AssetTags
{
#pragma region Input
	UE_DEFINE_GAMEPLAY_TAG(Asset_InputData, "Asset.InputData");
#pragma endregion

#pragma region UI
	UE_DEFINE_GAMEPLAY_TAG(Asset_UI_WidgetData, "Asset.UI.WidgetData");
	UE_DEFINE_GAMEPLAY_TAG(Asset_UI_LoadingData, "Asset.UI.LoadingData");
#pragma endregion

#pragma region DataTable
	UE_DEFINE_GAMEPLAY_TAG(Asset_DataTableData, "Asset.DataTableData");
#pragma endregion

#pragma region Pal
	UE_DEFINE_GAMEPLAY_TAG(Asset_Pal_Registry, "Asset.Pal.Registry");
	UE_DEFINE_GAMEPLAY_TAG(Asset_Ability_Pal, "Asset.Ability.Pal");
#pragma endregion
}

namespace DC1DataTableTags
{
	UE_DEFINE_GAMEPLAY_TAG(DataTable_UI_LoadingTip, "DataTable.UI.LoadingTip");
	UE_DEFINE_GAMEPLAY_TAG(DataTable_UI_TechTree, "DataTable.UI.TechTree");
}