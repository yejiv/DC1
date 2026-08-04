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
	UE_DEFINE_GAMEPLAY_TAG(Asset_UIData, "Asset.UIData");
#pragma endregion
}