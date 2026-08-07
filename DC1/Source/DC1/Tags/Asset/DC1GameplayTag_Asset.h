#pragma once

#include "NativeGameplayTags.h"

namespace DC1Labels
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Label_Preload);
}

namespace DC1AssetTags
{
#pragma region Input
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_InputData);
#pragma endregion

#pragma region UI
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_UI_WidgetData);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_UI_LoadingData);
#pragma endregion

#pragma region DataTable
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Asset_DataTableData);
#pragma endregion
}


namespace DC1DataTableTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(DataTable_UI_LoadingTip);
}