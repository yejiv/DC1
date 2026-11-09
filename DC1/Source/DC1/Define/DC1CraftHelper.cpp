#include "Define/DC1CraftHelper.h"

#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Base/DC1BaseRegistryComponent.h"
#include "Component/Inventory/Player/DC1PlayerInventoryComponent.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "GameMode/DC1GameState.h"

namespace
{
	void GetCraftInventories(const ADC1PlayerState* InPlayerState, TArray<UDC1InventoryComponentBase*>& OutInventories)
	{
		if (false == ::IsValid(InPlayerState))
		{
			return;
		}

		UDC1PlayerInventoryComponent* PlayerInventory = InPlayerState->GetInventoryComponent();
		if (true == ::IsValid(PlayerInventory))
		{
			OutInventories.Add(PlayerInventory);
		}

		const APawn* Pawn = InPlayerState->GetPawn();
		if (false == ::IsValid(Pawn))
		{
			return;
		}

		const ADC1GameState* GS = ADC1GameState::Get(InPlayerState);
		const ADC1PalBoxActor* PalBox = (true == ::IsValid(GS)) ? GS->FindBaseAt(Pawn->GetActorLocation()) : nullptr;
		if (false == ::IsValid(PalBox))
		{
			return;
		}

		UDC1BaseRegistryComponent* BaseRegisterComp = PalBox->GetRegistryComponent();
		if (false == ::IsValid(BaseRegisterComp))
		{
			return;
		}

		BaseRegisterComp->GetChestInventories(OutInventories);
	}

	int32 SumItemCount(const TArray<UDC1InventoryComponentBase*>& InInventories, FName InItemID)
	{
		int32 TotalCount = 0;

		for (const UDC1InventoryComponentBase* Inventory : InInventories)
		{
			TotalCount += Inventory->GetItemCount(InItemID);
		}

		return TotalCount;
	}
}

// 플레이어 인벤토리 + 거점 내부라면 거점 상자 포함 재료 개수
int32 DC1CraftHelper::GetCraftItemCount(const ADC1PlayerState* InPlayerState, FName InItemID)
{
	TArray<UDC1InventoryComponentBase*> Inventories;
	GetCraftInventories(InPlayerState, Inventories);

	return SumItemCount(Inventories, InItemID);
}
// 재료 차감 ( 서버 ) - 플레이어 인벤토리 먼저
bool DC1CraftHelper::RemoveCraftItem(ADC1PlayerState* InPlayerState, FName InItemID, int32 InCount)
{
	if (false == ::IsValid(InPlayerState) || false == InPlayerState->HasAuthority() || true == InItemID.IsNone() || InCount <= 0)
	{
		return false;
	}

	TArray<UDC1InventoryComponentBase*> Inventories;
	GetCraftInventories(InPlayerState, Inventories);

	if (SumItemCount(Inventories, InItemID) < InCount)
	{
		return false;
	}

	int32 TotalRemoveCount = InCount;

	for (UDC1InventoryComponentBase* Inventory : Inventories)
	{
		const int32 RemoveCount = FMath::Min(TotalRemoveCount, Inventory->GetItemCount(InItemID));
		if (RemoveCount <= 0)
		{
			continue;
		}

		Inventory->RemoveItemByID(InItemID, RemoveCount);
		TotalRemoveCount -= RemoveCount;

		if (TotalRemoveCount <= 0)
		{
			break;
		}
	}

	return true;
}

bool DC1CraftHelper::HasCraftCosts(const ADC1PlayerState* InPlayerState, const TArray<FDC1ItemCost>& InCosts)
{
	TArray<UDC1InventoryComponentBase*> Inventories;
	GetCraftInventories(InPlayerState, Inventories);

	for (const FDC1ItemCost& Cost : InCosts)
	{
		if (Cost.Count <= 0)
		{
			continue;
		}

		if (SumItemCount(Inventories, Cost.ItemID) < Cost.Count)
		{
			return false;
		}
	}

	return true;
}

bool DC1CraftHelper::RemoveCraftCosts(ADC1PlayerState* InPlayerState, const TArray<FDC1ItemCost>& InCosts)
{
	if (false == ::IsValid(InPlayerState) || false == InPlayerState->HasAuthority())
	{
		return false;
	}

	if (false == HasCraftCosts(InPlayerState, InCosts))
	{
		return false;
	}

	for (const FDC1ItemCost& Cost : InCosts)
	{
		if (Cost.Count <= 0)
		{
			continue;
		}

		RemoveCraftItem(InPlayerState, Cost.ItemID, Cost.Count);
	}

	return true;
}
