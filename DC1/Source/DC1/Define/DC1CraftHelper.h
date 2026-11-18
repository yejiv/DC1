#pragma once

#include "CoreMinimal.h"

class ADC1PlayerState;
class UDC1InventoryComponentBase;

struct FDC1ItemCost;

namespace DC1CraftHelper
{
	void	GetCraftInventories(const ADC1PlayerState* InPlayerState, TArray<UDC1InventoryComponentBase*>& OutInventories);

	int32	GetCraftItemCount(const ADC1PlayerState* InPlayerState, FName InItemID);

	bool	RemoveCraftItem(ADC1PlayerState* InPlayerState, FName InItemID, int32 InCount);

	bool	HasCraftCosts(const ADC1PlayerState* InPlayerState, const TArray<FDC1ItemCost>& InCosts);

	bool	RemoveCraftCosts(ADC1PlayerState* InPlayerState, const TArray<FDC1ItemCost>& InCosts);
}
