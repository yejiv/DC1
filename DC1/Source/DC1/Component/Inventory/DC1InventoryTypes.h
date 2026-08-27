#pragma once

#include "CoreMinimal.h"
#include "Enum/DC1Enums.h"
#include "DC1InventoryTypes.generated.h"

USTRUCT(BlueprintType)
struct FDC1InventorySlotInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName ItemID = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	int32 StackCount = 0;

	bool IsEmpty() const { return ItemID == NAME_None || StackCount <= 0; }
	void Clear() { ItemID = NAME_None; StackCount = 0; }
};

// 레어도별 색상 매핑
namespace DC1InventoryRarityColor
{
	FORCEINLINE FLinearColor GetRarityColor(EDC1ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case EDC1ItemRarity::Common:
			return FLinearColor(0.0f, 0.0f, 0.0f, 0.7f);

		case EDC1ItemRarity::Uncommon:
			return FLinearColor(0.026f, 0.604f, 0.168f, 0.7f);

		case EDC1ItemRarity::Rare:
			return FLinearColor(0.034f, 0.319f, 0.701f, 0.7f);

		case EDC1ItemRarity::Epic:
			return FLinearColor(0.332f, 0.095f, 0.473f, 0.7f);

		case EDC1ItemRarity::Legendary:
			return FLinearColor(0.791f, 0.216f, 0.017f, 0.7f);

		default:
			return FLinearColor(0.0f, 0.0f, 0.0f, 0.7f);
		}
	}
}
