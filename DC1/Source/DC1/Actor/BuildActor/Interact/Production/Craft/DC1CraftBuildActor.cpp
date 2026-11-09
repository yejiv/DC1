// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Interact/Production/Craft/DC1CraftBuildActor.h"

#include "Character/DC1Character.h"
#include "Character/Player/DC1PlayerController.h"
#include "Character/Player/DC1PlayerState.h"

#include "Item/Pickup/DC1PickupItem.h"

#include "Component/DC1TechTreeComponent.h"
#include "Component/Interact/DC1InteractComponent.h"
#include "Component/Inventory/Player/DC1PlayerInventoryComponent.h"

#include "Data/DataTable/DC1DataTableTypes.h"

#include "Define/DC1CraftHelper.h"
#include "Define/DC1ItemHelper.h"

#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Tags/UI/DC1GameplayTag_UI.h"
#include "UI/Craft/DC1CraftScreenWidget.h"

namespace
{
	void GiveOrDrop(UDC1PlayerInventoryComponent* InInventory, const AActor* InDropAt, FName InItemID, int32 InCount)
	{
		if (InCount <= 0)
		{
			return;
		}

		if (true == InInventory->AddItem(InItemID, InCount))
		{
			return;
		}

		ADC1PickupItem::SpawnDrop(InDropAt->GetWorld(), InItemID, InCount, InDropAt->GetActorLocation());
	}

	bool HasOptionKey(const TArray<FDC1InteractOption>& InOptions, EDC1InteractKey InKey)
	{
		return InOptions.ContainsByPredicate([InKey](const FDC1InteractOption& Option) { return Option.Key == InKey; });
	}
}

ADC1CraftBuildActor::ADC1CraftBuildActor()
{
	RecipeOptions.Emplace(EDC1InteractKey::F, NSLOCTEXT("Interact", "Craft", "제작"), EDC1InteractInputType::Press, FGameplayTag::EmptyTag);
	CancelOptions.Emplace(EDC1InteractKey::C, NSLOCTEXT("Interact", "Cancel", "취소"), EDC1InteractInputType::HoldConfirm, FGameplayTag::EmptyTag);
}

void ADC1CraftBuildActor::GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const
{
	Super::GetInteractOptions(OutOptions);

	if (false == IsCompleted())
	{
		return;
	}

	// 제작 중이 아니라면 제작 옵션으로 변경
	if (true == ProductItemID.IsNone())
	{
		OutOptions = RecipeOptions;
		return;
	}

	// 제작 중이라면 취소 옵션 추가
	OverrideOptions(OutOptions, CancelOptions);
}

bool ADC1CraftBuildActor::CanShowRecipe(const FDC1ItemRecipeRow& InRecipe, const ADC1PlayerState* InPlayerState) const
{
	if (false == ::IsValid(InPlayerState))
	{
		return false;
	}

	const FDC1ItemRow* ProductRow = DC1ItemHelper::FindItemRow(InRecipe.ProductItemID);
	if (nullptr == ProductRow)
	{
		return false;
	}

	// 제작대 제작 가능 확인
	if (false == ProductRow->CategoryTag.MatchesAny(CraftableCategoryTags))
	{
		return false;
	}

	// 제작대 등급 확인
	if (ProductRow->Rank > CraftableRank)
	{
		return false;
	}

	// 태크 트리 해금 여부
	if (true == InRecipe.RequiredTechTag.IsValid())
	{
		UDC1TechTreeComponent* TechComp = InPlayerState->GetTechTreeComponent();
		if (true == ::IsValid(TechComp) && false == TechComp->IsTechUnlocked(InRecipe.RequiredTechTag))
		{
			return false;
		}
	}
	// 레시피 보유 여부
	if (false == InRecipe.RequiredSchematicItemID.IsNone())
	{
		if (DC1CraftHelper::GetCraftItemCount(InPlayerState, InRecipe.RequiredSchematicItemID) <= 0)
		{
			return false;
		}
	}

	return true;
}

int32 ADC1CraftBuildActor::GetCraftableCount(const FDC1ItemRecipeRow& InRecipe, const ADC1PlayerState* InPlayerState) const
{
	if (false == ::IsValid(InPlayerState))
	{
		return 0;
	}

	int32 Result = MaxCraftCount;

	for (const FDC1RecipeItem& RequiredItem : InRecipe.RequiredItems)
	{
		if (RequiredItem.Count <= 0)
		{
			continue;
		}

		int32 TotalCount = DC1CraftHelper::GetCraftItemCount(InPlayerState, RequiredItem.ItemID);
		if (TotalCount <= 0 )
		{
			return 0;
		}

		int32 CraftableCount = FMath::DivideAndRoundDown(TotalCount, RequiredItem.Count);
		if (CraftableCount <= 0)
		{
			return 0;
		}

		Result = FMath::Min(Result, CraftableCount);
	}

	return Result;
}

void ADC1CraftBuildActor::RegisterRecipe(ADC1Character* InRegister, FName InRecipeID, int32 InCraftCount)
{
	if (false == HasAuthority() || false == ::IsValid(InRegister)
		|| false == IsCompleted())
	{
		return;
	}

	// 제작 중
	if (false == ProductItemID.IsNone())
	{
		return;
	}

	ADC1PlayerState* PlayerState = InRegister->GetPlayerState<ADC1PlayerState>();
	if (false == ::IsValid(PlayerState))
	{
		return;
	}

	FDC1ItemRecipeRow Recipe = {};
	if (false == DC1RecipeHelper::FindRecipeRow(InRecipeID, Recipe))
	{
		return;
	}

	if (false == CanShowRecipe(Recipe, PlayerState))
	{
		return;
	}

	if (InCraftCount <= 0 || InCraftCount > GetCraftableCount(Recipe, PlayerState))
	{
		return;
	}

	for (const FDC1RecipeItem& RequiredItem : Recipe.RequiredItems)
	{
		if (false == DC1CraftHelper::RemoveCraftItem(PlayerState, RequiredItem.ItemID, RequiredItem.Count * InCraftCount))
		{
			UE_LOG(LogTemp, Warning, TEXT("재료 중복 등록 : Recipe %s"), *InRecipeID.ToString());
			return;
		}
	}

	CurrentRecipeID = InRecipeID;
	ProductItemID = Recipe.ProductItemID;
	RequiredWorkPerItem = Recipe.RequiredWork;
	ProductCountPerWork = Recipe.ProductCount;
	MaxStoredCount = ProductCountPerWork * InCraftCount;

	CurrentProductionWork = 0.f;
	ProductionProgress = 0.f;

	// Standalone
	ApplyProductItemID();
}

void ADC1CraftBuildActor::CancelRecipe(ADC1Character* InCanceller)
{
	if (false == HasAuthority() || false == ::IsValid(InCanceller) || true == ProductItemID.IsNone())
	{
		return;
	}

	ADC1PlayerState* PlayerState = InCanceller->GetPlayerState<ADC1PlayerState>();
	if (false == ::IsValid(PlayerState))
	{
		return;
	}

	UDC1PlayerInventoryComponent* InventoryComp = PlayerState->GetInventoryComponent();
	if (false == ::IsValid(InventoryComp))
	{
		return;
	}

	FDC1ItemRecipeRow Recipe = {};
	if (false == DC1RecipeHelper::FindRecipeRow(CurrentRecipeID, Recipe))
	{
		return;
	}

	// 완성품 + 남은 재료 반환
	const int32 RemainCraftCount = (MaxStoredCount - StoredCount) / FMath::Max(ProductCountPerWork, 1);

	// 남은 재료 반환
	for (const FDC1RecipeItem& RequiredItem : Recipe.RequiredItems)
	{
		GiveOrDrop(InventoryComp, InCanceller, RequiredItem.ItemID, RequiredItem.Count * RemainCraftCount);
	}
	// 완성품 반환
	GiveOrDrop(InventoryComp, InCanceller, ProductItemID, StoredCount);

	ClearWorkers();
	ClearPalWorkers();
	SetStoredCount(0);
	ClearRecipeOrder();
}

void ADC1CraftBuildActor::OnCompletedInteract(ADC1Character* InInteractOwner, EDC1InteractKey InKey)
{
	// 제작 중인 아이템 없음 + F ( 제작 )
	if (true == ProductItemID.IsNone() && true == HasOptionKey(RecipeOptions, InKey))
	{
		UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
		if (false == ::IsValid(UIManager))
		{
			return;
		}

		UDC1CraftScreenWidget* CraftScreen = Cast<UDC1CraftScreenWidget>(UIManager->PushScreen(DC1UITags::UI_Craft_Recipe));
		if (true == ::IsValid(CraftScreen))
		{
			CraftScreen->OpenStation(this);
		}
		return;
	}

	// 제작 중인 아이템 있음 + C ( 취소 )
	if (false == ProductItemID.IsNone() && true == HasOptionKey(CancelOptions, InKey))
	{
		ADC1PlayerController* PC = InInteractOwner->GetController<ADC1PlayerController>();
		UDC1InteractComponent* InteractComp = (true == ::IsValid(PC)) ? PC->GetInteractComponent() : nullptr;
		if (true == ::IsValid(InteractComp))
		{
			InteractComp->RequestCancelRecipe(this);
		}
		return;
	}

	Super::OnCompletedInteract(InInteractOwner, InKey);
}

void ADC1CraftBuildActor::OnProductCollected(int32 InCollectedCount)
{
	MaxStoredCount -= InCollectedCount;

	if (MaxStoredCount <= 0)
	{
		ClearRecipeOrder();
	}
}

void ADC1CraftBuildActor::ClearRecipeOrder()
{
	CurrentRecipeID = NAME_None;
	ProductItemID = NAME_None;
	ProductCountPerWork = 1;
	CurrentProductionWork = 0.f;
	ProductionProgress = 0.f;

	ApplyProductItemID();
}
