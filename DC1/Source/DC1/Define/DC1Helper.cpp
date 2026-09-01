#include "DC1Helper.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Data/DA/DataTable/DC1DataTableData.h"

#pragma region DataTable Helper

const UDataTable* DC1DataTableHelper::GetDataTable(const FGameplayTag& InTableTag)
{
	const UDC1DataTableData* DataTable = UDC1AssetManager::GetAssetByTag<UDC1DataTableData>(DC1AssetTags::Asset_DataTableData);
	if (false == ::IsValid(DataTable))
	{
		UE_LOG(LogTemp, Warning, TEXT("DC1DataTableHelper :: Can't Find DataTable Asset from AssetManager"));
		return nullptr;
	}

	const UDataTable* OutTable = DataTable->GetDataTable(InTableTag);
	if (false == ::IsValid(OutTable))
	{
		UE_LOG(LogTemp, Warning, TEXT("DC1DataTableHelper :: Can't Find DataTable from DataTableAsset : %s"), *InTableTag.ToString());
		return nullptr;
	}

    return OutTable;
}

#pragma endregion