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
			// 일반: 부드러운 차콜 그레이 (#7F8C8D)
			return FLinearColor(0.216f, 0.275f, 0.279f, 1.0f);

		case EDC1ItemRarity::Uncommon:
			// 고급: 에메랄드 그린 (#2ECC71)
			return FLinearColor(0.026f, 0.604f, 0.168f, 1.0f);

		case EDC1ItemRarity::Rare:
			// 희귀: 비비드 스카이 블루 (#3498DB)
			return FLinearColor(0.034f, 0.319f, 0.701f, 1.0f);

		case EDC1ItemRarity::Epic:
			// 영웅: 딥 바이올렛 (#9B59B6)
			return FLinearColor(0.332f, 0.095f, 0.473f, 1.0f);

		case EDC1ItemRarity::Legendary:
			// 전설: 앰버 오렌지 / 황금빛 (#E67E22)
			return FLinearColor(0.791f, 0.216f, 0.017f, 1.0f);

		default:
			return FLinearColor(0.216f, 0.275f, 0.279f, 1.0f); // Fallback: Common
		}
	}
}
