#pragma once

#include "CoreMinimal.h"
#include "Enum/DC1Enums.h"
#include "DC1InventoryTypes.generated.h"

USTRUCT(BlueprintType)
struct FDC1InventorySlot
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
			return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // 회색
		case EDC1ItemRarity::Uncommon:
			return FLinearColor(0.2f, 0.8f, 0.2f, 1.0f); // 초록색
		case EDC1ItemRarity::Rare:
			return FLinearColor(0.2f, 0.5f, 1.0f, 1.0f); // 파란색
		case EDC1ItemRarity::Epic:
			return FLinearColor(0.8f, 0.2f, 0.8f, 1.0f); // 보라색
		case EDC1ItemRarity::Legendary:
			return FLinearColor(1.0f, 0.8f, 0.0f, 1.0f); // 황금색
		default:
			return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // 기본값: 회색
		}
	}
}
