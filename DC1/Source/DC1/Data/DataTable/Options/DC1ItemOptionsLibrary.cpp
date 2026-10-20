// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DataTable/Options/DC1ItemOptionsLibrary.h"

#include "Data/DataTable/DC1DataTableTypes.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/Item/DC1GameplayTag_Item.h"

#if WITH_EDITOR
#include "AssetRegistry/IAssetRegistry.h"
#include "Data/DA/DataTable/DC1DataTableData.h"
#endif

#if WITH_EDITOR
namespace
{
	// AssetManager는 게임 시작 전엔 비어 있어서 DA_DataTable을 AssetRegistry로 직접 찾음
	const UDataTable* FindDataTableInEditor(const FGameplayTag& InTableTag)
	{
		TArray<FAssetData> Assets;
		IAssetRegistry::GetChecked().GetAssetsByClass(UDC1DataTableData::StaticClass()->GetClassPathName(), Assets);

		for (const FAssetData& AssetData : Assets)
		{
			const UDC1DataTableData* DataTableData = Cast<UDC1DataTableData>(AssetData.GetAsset());
			if (true == ::IsValid(DataTableData))
			{
				if (const UDataTable* DataTable = DataTableData->GetDataTable(InTableTag))
				{
					return DataTable;
				}
			}
		}

		return nullptr;
	}

	void CollectItemRows(const FGameplayTag& InTableTag, TArray<const FDC1ItemRow*>& OutRows)
	{
		const UDataTable* DataTable = FindDataTableInEditor(InTableTag);
		if (false == ::IsValid(DataTable))
		{
			return;
		}

		// 귀중품 테이블( FDC1ValuableItemRow )도 FDC1ItemRow 자식이라 같이 처리
		const UScriptStruct* RowStruct = DataTable->GetRowStruct();
		if (nullptr == RowStruct || false == RowStruct->IsChildOf(FDC1ItemRow::StaticStruct()))
		{
			return;
		}

		for (const TPair<FName, uint8*>& RowPair : DataTable->GetRowMap())
		{
			OutRows.Add(reinterpret_cast<const FDC1ItemRow*>(RowPair.Value));
		}
	}

	void CollectAllItemRows(TArray<const FDC1ItemRow*>& OutRows)
	{
		CollectItemRows(DC1DataTableTags::DataTable_Item, OutRows);
		CollectItemRows(DC1DataTableTags::DataTable_ValuableItem, OutRows);
	}

	// ValueString은 저장 값, DisplayName은 콤보박스에 보이는 글자
	TArray<FPropertyTextFName> MakeOptions(const FGameplayTag& InCategoryFilter)
	{
		TArray<const FDC1ItemRow*> ItemRows;
		CollectAllItemRows(ItemRows);

		// ItemID가 4자리 분류 번호라 문자열 정렬이 곧 분류 순
		ItemRows.Sort([](const FDC1ItemRow& A, const FDC1ItemRow& B) { return A.ItemID.LexicalLess(B.ItemID); });

		TArray<FPropertyTextFName> Options;
		Options.Reserve(ItemRows.Num());

		for (const FDC1ItemRow* ItemRow : ItemRows)
		{
			if (true == InCategoryFilter.IsValid() && false == ItemRow->CategoryTag.MatchesTag(InCategoryFilter))
			{
				continue;
			}

			FPropertyTextFName& Option = Options.AddDefaulted_GetRef();
			Option.ValueString = ItemRow->ItemID;
			Option.DisplayName = FText::Format(INVTEXT("{0}  {1}"), FText::FromName(ItemRow->ItemID), ItemRow->ItemName);
		}

		return Options;
	}
}
#endif

TArray<FPropertyTextFName> UDC1ItemOptionsLibrary::GetItemIDOptions()
{
#if WITH_EDITOR
	return MakeOptions(FGameplayTag::EmptyTag);
#else
	return {};
#endif
}

TArray<FPropertyTextFName> UDC1ItemOptionsLibrary::GetSchematicItemIDOptions()
{
#if WITH_EDITOR
	TArray<FPropertyTextFName> Options = MakeOptions(DC1ItemTags::Item_Type_Schematic);

	FPropertyTextFName NoneOption;
	NoneOption.ValueString = NAME_None;
	NoneOption.DisplayName = INVTEXT("None  (설계도 필요 없음)");
	Options.Insert(NoneOption, 0);

	return Options;
#else
	return {};
#endif
}

#if WITH_EDITOR
const FDC1ItemRow* UDC1ItemOptionsLibrary::FindItemRowInEditor(FName InItemID)
{
	if (InItemID.IsNone())
	{
		return nullptr;
	}

	// 편집할 때만 불려서 캐시 없이 매번 읽음
	TArray<const FDC1ItemRow*> ItemRows;
	CollectAllItemRows(ItemRows);

	const FDC1ItemRow* const* Found = ItemRows.FindByPredicate([InItemID](const FDC1ItemRow* Row) { return InItemID == Row->ItemID; });
	return (nullptr != Found) ? *Found : nullptr;
}
#endif
