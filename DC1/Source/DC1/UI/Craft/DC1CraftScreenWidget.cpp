// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Craft/DC1CraftScreenWidget.h"

#include "UI/Button/AnimButton/Slot/DC1RecipeButton.h"
#include "UI/Craft/DC1CraftSelectWidget.h"

#include "Actor/BuildActor/Interact/Production/Craft/DC1CraftBuildActor.h"
#include "Character/Player/DC1PlayerController.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Interact/DC1InteractComponent.h"

#include "Define/DC1Helper.h"
#include "Define/DC1ItemHelper.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Components/DynamicEntryBox.h"
#include "Components/TextBlock.h"

UDC1CraftScreenWidget::UDC1CraftScreenWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CloseKeys.Keys = { EKeys::Escape, EKeys::F };
}

void UDC1CraftScreenWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (true == ::IsValid(CraftSelectWidget))
	{
		CraftSelectWidget->OnCraftRequested.AddUObject(this, &ThisClass::HandleCraftRequested);
	}
}

void UDC1CraftScreenWidget::OpenStation(ADC1CraftBuildActor* InStation)
{
	if (true == Station.IsValid())
	{
		Station->OnProductionChanged.RemoveAll(this);
	}

	Station = InStation;

	if (false == ::IsValid(InStation))
	{
		return;
	}

	InStation->OnProductionChanged.AddUObject(this, &ThisClass::HandleProductionChanged);

	CacheDataRows();

	if (true == ::IsValid(Text_StationName))
	{
		Text_StationName->SetText(StationNames.FindRef(InStation->GetBuildActorData().BuildTag));
	}

	BuildRecipeList();

	if (DefaultTabTag == CurrentTab || false == SelectTab(DefaultTabTag))
	{
		ApplyTabFilter();
	}
}

void UDC1CraftScreenWidget::CacheDataRows()
{
	if (true == CachedRecipeRows.IsEmpty())
	{
		DC1DataTableHelper::GetAllRows<FDC1ItemRecipeRow>(DC1DataTableTags::DataTable_ItemRecipe, CachedRecipeRows);
	}

	if (false == StationNames.IsEmpty())
	{
		return;
	}

	TArray<FDC1BuildRow> BuildRows;
	if (false == DC1DataTableHelper::GetAllRows<FDC1BuildRow>(DC1DataTableTags::DataTable_UI_Build, BuildRows))
	{
		return;
	}

	for (const FDC1BuildRow& BuildRow : BuildRows)
	{
		StationNames.Add(BuildRow.BuildTag, BuildRow.DisplayName);
	}
}

void UDC1CraftScreenWidget::BuildRecipeList()
{
	ClearSelection();

	if (true == ::IsValid(RecipeListBox))
	{
		RecipeListBox->Reset<UDC1RecipeButton>([this](UDC1RecipeButton& InEntry)
			{
				InEntry.OnRecipeClicked.RemoveAll(this);
			});
	}

	RecipeButtons.Reset();
	ShownRecipes.Reset();

	const ADC1CraftBuildActor* CurrentStation = Station.Get();
	const ADC1PlayerState* PlayerState = GetOwningPlayerState<ADC1PlayerState>();
	if (false == ::IsValid(CurrentStation) || false == ::IsValid(PlayerState) || false == ::IsValid(RecipeListBox))
	{
		return;
	}

	for (const FDC1ItemRecipeRow& RecipeRow : CachedRecipeRows)
	{
		if (false == CurrentStation->CanShowRecipe(RecipeRow, PlayerState))
		{
			continue;
		}

		UDC1RecipeButton* Entry = RecipeListBox->CreateEntry<UDC1RecipeButton>();
		if (false == ::IsValid(Entry))
		{
			continue;
		}

		ShownRecipes.Add(RecipeRow.RecipeID, &RecipeRow);
		RecipeButtons.Add(RecipeRow.RecipeID, Entry);

		Entry->SetRecipe(RecipeRow, CurrentStation->GetCraftableCount(RecipeRow, PlayerState) > 0);
		Entry->OnRecipeClicked.AddUObject(this, &ThisClass::SelectRecipe);
	}
}

void UDC1CraftScreenWidget::OnTabSelected(const FGameplayTag& InTab)
{
	Super::OnTabSelected(InTab);

	ApplyTabFilter();
}

void UDC1CraftScreenWidget::ApplyTabFilter()
{
	for (const TPair<FName, TObjectPtr<UDC1RecipeButton>>& Pair : RecipeButtons)
	{
		UDC1RecipeButton* RecipeButton = Pair.Value.Get();
		const FDC1ItemRecipeRow* Recipe = ShownRecipes.FindRef(Pair.Key);
		if (false == ::IsValid(RecipeButton) || nullptr == Recipe)
		{
			continue;
		}

		const bool IsVisible = IsRecipeInTab(*Recipe);
		RecipeButton->SetVisibility(IsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

		if (false == IsVisible && Pair.Key == SelectedRecipeID)
		{
			ClearSelection();
		}
	}
}

bool UDC1CraftScreenWidget::IsRecipeInTab(const FDC1ItemRecipeRow& InRecipe) const
{
	const FGameplayTagContainer* ItemTypes = TabItemTypes.Find(CurrentTab);
	if (nullptr == ItemTypes || true == ItemTypes->IsEmpty())
	{
		return true;
	}

	const FDC1ItemRow* ProductRow = DC1ItemHelper::FindItemRow(InRecipe.ProductItemID);
	if (nullptr == ProductRow)
	{
		return false;
	}

	return ProductRow->CategoryTag.MatchesAny(*ItemTypes);
}

void UDC1CraftScreenWidget::SelectRecipe(FName InRecipeID)
{
	if (InRecipeID == SelectedRecipeID)
	{
		return;
	}

	const FDC1ItemRecipeRow* Recipe = ShownRecipes.FindRef(InRecipeID);
	const ADC1CraftBuildActor* CurrentStation = Station.Get();
	const ADC1PlayerState* PlayerState = GetOwningPlayerState<ADC1PlayerState>();
	if (nullptr == Recipe || false == ::IsValid(CurrentStation) || false == ::IsValid(PlayerState))
	{
		return;
	}

	UDC1RecipeButton* PrevButton = RecipeButtons.FindRef(SelectedRecipeID);
	if (true == ::IsValid(PrevButton))
	{
		PrevButton->SetSelected(false);
	}

	SelectedRecipeID = InRecipeID;

	UDC1RecipeButton* NewButton = RecipeButtons.FindRef(SelectedRecipeID);
	if (true == ::IsValid(NewButton))
	{
		NewButton->SetSelected(true);
	}

	if (true == ::IsValid(CraftSelectWidget))
	{
		CraftSelectWidget->SetRecipe(*Recipe, CurrentStation->GetCraftableCount(*Recipe, PlayerState));
	}
}

void UDC1CraftScreenWidget::ClearSelection()
{
	UDC1RecipeButton* PrevButton = RecipeButtons.FindRef(SelectedRecipeID);
	if (true == ::IsValid(PrevButton))
	{
		PrevButton->SetSelected(false);
	}

	SelectedRecipeID = NAME_None;

	if (true == ::IsValid(CraftSelectWidget))
	{
		CraftSelectWidget->ClearRecipe();
	}
}

void UDC1CraftScreenWidget::HandleCraftRequested(FName InRecipeID, int32 InCount)
{
	ADC1CraftBuildActor* CurrentStation = Station.Get();
	if (false == ::IsValid(CurrentStation) || true == InRecipeID.IsNone() || InCount <= 0)
	{
		return;
	}

	ADC1PlayerController* PC = GetOwningPlayer<ADC1PlayerController>();
	UDC1InteractComponent* InteractComp = (true == ::IsValid(PC)) ? PC->GetInteractComponent() : nullptr;
	if (false == ::IsValid(InteractComp))
	{
		return;
	}

	InteractComp->RequestRegisterRecipe(CurrentStation, InRecipeID, InCount);

	PopScreen();
}

void UDC1CraftScreenWidget::HandleProductionChanged()
{
	const ADC1CraftBuildActor* CurrentStation = Station.Get();
	if (true == ::IsValid(CurrentStation) && true == CurrentStation->GetProductionID().IsNone())
	{
		return;
	}

	PopScreen();
}

void UDC1CraftScreenWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if (true == Station.IsValid())
	{
		Station->OnProductionChanged.RemoveAll(this);
	}

	Station.Reset();
}

void UDC1CraftScreenWidget::CloseWidget(const FKey& InKey)
{
	if (false == CloseKeys.Keys.Contains(InKey))
	{
		return;
	}

	PopScreen();
}
