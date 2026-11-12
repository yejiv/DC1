// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/Build/DC1HUDBuildMaterialList.h"

#include "Components/PanelWidget.h"
#include "TimerManager.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/Inventory/DC1InventoryComponentBase.h"
#include "Define/DC1CraftHelper.h"
#include "UI/Craft/DC1CraftMaterialEntryWidget.h"

void UDC1HUDBuildMaterialList::SetCosts(const TArray<FDC1ItemCost>& InCosts)
{
	Clear();

	CachedCosts = InCosts;

	if (CachedCosts.Num() > Slots.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1HUDBuildMaterialList :: SetCosts - Costs(%d) > Slots(%d)"), CachedCosts.Num(), Slots.Num());
	}

	RefreshOwnedCounts();
	RebindInventories();
	StartInventoryRecheck();
}

void UDC1HUDBuildMaterialList::SetAmounts(const TArray<FDC1ItemCost>& InCosts)
{
	Clear();

	if (InCosts.Num() > Slots.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1HUDBuildMaterialList :: SetAmounts - Costs(%d) > Slots(%d)"), InCosts.Num(), Slots.Num());
	}

	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		UDC1CraftMaterialEntryWidget* MaterialSlot = Slots[Index];
		if (false == ::IsValid(MaterialSlot) || false == InCosts.IsValidIndex(Index))
		{
			continue;
		}

		MaterialSlot->SetMaterialAmount(InCosts[Index].ItemID, InCosts[Index].Count);
	}
}

void UDC1HUDBuildMaterialList::Clear()
{
	StopInventoryRecheck();
	UnbindInventories();

	CachedCosts.Reset();

	for (UDC1CraftMaterialEntryWidget* MaterialSlot : Slots)
	{
		if (true == ::IsValid(MaterialSlot))
		{
			MaterialSlot->ClearMaterial();
		}
	}
}

void UDC1HUDBuildMaterialList::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Slots.Reset();

	if (false == ::IsValid(SlotBox))
	{
		return;
	}

	for (UWidget* Child : SlotBox->GetAllChildren())
	{
		UDC1CraftMaterialEntryWidget* MaterialSlot = Cast<UDC1CraftMaterialEntryWidget>(Child);
		if (true == ::IsValid(MaterialSlot))
		{
			MaterialSlot->ClearMaterial();
			Slots.Add(MaterialSlot);
		}
	}
}

void UDC1HUDBuildMaterialList::NativeDestruct()
{
	Clear();

	Super::NativeDestruct();
}

void UDC1HUDBuildMaterialList::RefreshOwnedCounts()
{
	const ADC1PlayerState* PlayerState = GetOwningPlayerState<ADC1PlayerState>();

	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		UDC1CraftMaterialEntryWidget* MaterialSlot = Slots[Index];
		if (false == ::IsValid(MaterialSlot))
		{
			continue;
		}

		if (false == CachedCosts.IsValidIndex(Index))
		{
			MaterialSlot->ClearMaterial();
			continue;
		}

		const FDC1ItemCost& Cost = CachedCosts[Index];
		MaterialSlot->SetMaterial(Cost.ItemID, DC1CraftHelper::GetCraftItemCount(PlayerState, Cost.ItemID), Cost.Count);
	}
}

void UDC1HUDBuildMaterialList::HandleInventoryUpdated()
{
	RefreshOwnedCounts();
}

void UDC1HUDBuildMaterialList::RebindInventories()
{
	TArray<UDC1InventoryComponentBase*> CurrentInventories;
	DC1CraftHelper::GetCraftInventories(GetOwningPlayerState<ADC1PlayerState>(), CurrentInventories);

	if (true == IsSameInventories(CurrentInventories))
	{
		return;
	}

	UnbindInventories();

	for (UDC1InventoryComponentBase* Inventory : CurrentInventories)
	{
		if (false == ::IsValid(Inventory))
		{
			continue;
		}

		Inventory->OnInventoryUpdated.AddUObject(this, &ThisClass::HandleInventoryUpdated);
		BoundInventories.Add(Inventory);
	}

	RefreshOwnedCounts();
}

void UDC1HUDBuildMaterialList::UnbindInventories()
{
	for (const TWeakObjectPtr<UDC1InventoryComponentBase>& WeakInventory : BoundInventories)
	{
		UDC1InventoryComponentBase* Inventory = WeakInventory.Get();
		if (nullptr != Inventory)
		{
			Inventory->OnInventoryUpdated.RemoveAll(this);
		}
	}

	BoundInventories.Reset();
}

bool UDC1HUDBuildMaterialList::IsSameInventories(const TArray<UDC1InventoryComponentBase*>& InInventories) const
{
	if (InInventories.Num() != BoundInventories.Num())
	{
		return false;
	}

	for (UDC1InventoryComponentBase* Inventory : InInventories)
	{
		if (false == BoundInventories.Contains(Inventory))
		{
			return false;
		}
	}

	return true;
}

void UDC1HUDBuildMaterialList::StartInventoryRecheck()
{
	UWorld* World = GetWorld();
	if (nullptr == World)
	{
		return;
	}

	World->GetTimerManager().SetTimer(InventoryRecheckTimer, this, &ThisClass::RebindInventories, InventoryRecheckInterval, true);
}

void UDC1HUDBuildMaterialList::StopInventoryRecheck()
{
	UWorld* World = GetWorld();
	if (nullptr != World)
	{
		World->GetTimerManager().ClearTimer(InventoryRecheckTimer);
	}

	InventoryRecheckTimer.Invalidate();
}
