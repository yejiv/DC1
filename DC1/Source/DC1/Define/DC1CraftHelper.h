#pragma once

#include "CoreMinimal.h"

class ADC1PlayerState;

namespace DC1CraftHelper
{
	int32	GetCraftItemCount(const ADC1PlayerState* InPlayerState, FName InItemID);

	bool	RemoveCraftItem(ADC1PlayerState* InPlayerState, FName InItemID, int32 InCount);
}
