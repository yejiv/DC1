// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DataTable/DC1DataTableTypes.h"

#if WITH_EDITOR
#include "Data/DataTable/Options/DC1ItemOptionsLibrary.h"
#endif

#pragma region ItemRecipe

#if WITH_EDITOR
namespace
{
	// DC1ItemHelper는 게임 시작 전엔 동작하지 않음
	FString MakeItemLabel(FName InItemID)
	{
		if (InItemID.IsNone())
		{
			return TEXT("(비어 있음)");
		}

		const FDC1ItemRow* ItemRow = UDC1ItemOptionsLibrary::FindItemRowInEditor(InItemID);
		if (nullptr == ItemRow)
		{
			return FString::Printf(TEXT("⚠ 없는 아이템 %s"), *InItemID.ToString());
		}

		return ItemRow->ItemName.ToString();
	}
}
#endif

// 행 하나를 고치면 그 행만, 행 추가·삭제 등은 전체 행이 호출됨
void FDC1ItemRecipeRow::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	RefreshEditorLabels();
}

void FDC1ItemRecipeRow::OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, TArray<FString>& OutCollectedImportProblems)
{
	RefreshEditorLabels();
}

// 아이템 이름을 바꿔도 레시피 테이블을 다시 편집해야 반영됨
void FDC1ItemRecipeRow::RefreshEditorLabels()
{
#if WITH_EDITOR
	TArray<FString> RequiredLabels;
	RequiredLabels.Reserve(RequiredItems.Num());

	for (FDC1RecipeItem& RequiredItem : RequiredItems)
	{
		RequiredItem.EditorLabel = FString::Printf(TEXT("%s ×%d"), *MakeItemLabel(RequiredItem.ItemID), RequiredItem.Count);
		RequiredLabels.Add(RequiredItem.EditorLabel);
	}

	EditorSummary = FString::Printf(TEXT("%s ×%d ← %s ( 작업량 %g )"),
		*MakeItemLabel(ProductItemID), ProductCount, *FString::Join(RequiredLabels, TEXT(", ")), RequiredWork);

	if (true == RequiredTechTag.IsValid())
	{
		EditorSummary += FString::Printf(TEXT(" · 해금 %s"), *RequiredTechTag.ToString());
	}

	if (false == RequiredSchematicItemID.IsNone())
	{
		EditorSummary += FString::Printf(TEXT(" · 설계도 %s"), *MakeItemLabel(RequiredSchematicItemID));
	}
#endif
}

#pragma endregion
