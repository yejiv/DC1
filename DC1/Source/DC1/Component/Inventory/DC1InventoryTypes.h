#pragma once

#include "CoreMinimal.h"
#include "Define/DC1Enums.h"
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

	bool operator==(const FDC1InventorySlotInfo& Other) const
	{
		return ItemID == Other.ItemID && StackCount == Other.StackCount;
	}
};

// 장비 칸 배치 (EquipmentSlots 인덱스 순서: 무기6 / 머리 / 갑옷 / 방패 / 글라이더 / 장신구4 / 음식5)
namespace DC1EquipSlotLayout
{
	inline constexpr EDC1EquipSlotType SlotTypes[] =
	{
		EDC1EquipSlotType::Weapon, EDC1EquipSlotType::Weapon, EDC1EquipSlotType::Weapon, EDC1EquipSlotType::Weapon, EDC1EquipSlotType::Weapon, EDC1EquipSlotType::Weapon,
		EDC1EquipSlotType::Head,
		EDC1EquipSlotType::Body,
		EDC1EquipSlotType::Shield,
		EDC1EquipSlotType::Glider,
		EDC1EquipSlotType::Accessory, EDC1EquipSlotType::Accessory, EDC1EquipSlotType::Accessory, EDC1EquipSlotType::Accessory,
		EDC1EquipSlotType::Food, EDC1EquipSlotType::Food, EDC1EquipSlotType::Food, EDC1EquipSlotType::Food, EDC1EquipSlotType::Food,
	};

	inline constexpr int32 SlotCount = UE_ARRAY_COUNT(SlotTypes);

	// 처음부터 열려 있는 칸 수 (나머지는 잠김, 음식은 먹이 주머니로 열림)
	inline constexpr int32 DefaultUnlockedWeaponCount = 4;
	inline constexpr int32 DefaultUnlockedAccessoryCount = 2;

	// 해당 타입의 전체 칸 수를 구하는 함수
	FORCEINLINE int32 GetTypeSlotCount(const EDC1EquipSlotType InSlotType)
	{
		int32 Count = 0;
		for (const EDC1EquipSlotType SlotType : SlotTypes)
		{
			if (InSlotType == SlotType)
			{
				++Count;
			}
		}
		return Count;
	}

	// 장비 칸 인덱스로 칸 타입을 얻는 함수 (범위 밖이면 None)
	FORCEINLINE EDC1EquipSlotType GetSlotType(const int32 InEquipIndex)
	{
		return (0 <= InEquipIndex && InEquipIndex < SlotCount) ? SlotTypes[InEquipIndex] : EDC1EquipSlotType::None;
	}

	// 같은 타입 칸 중 몇 번째인지 구하는 함수 (무기 칸 -> WeaponSlots 인덱스, 없으면 -1)
	FORCEINLINE int32 GetLocalIndex(const int32 InEquipIndex)
	{
		const EDC1EquipSlotType SlotType = GetSlotType(InEquipIndex);
		if (EDC1EquipSlotType::None == SlotType)
		{
			return -1;
		}

		int32 LocalIndex = 0;
		for (int32 Index = 0; Index < InEquipIndex; ++Index)
		{
			if (SlotType == SlotTypes[Index])
			{
				++LocalIndex;
			}
		}
		return LocalIndex;
	}

	// GetLocalIndex의 역방향 함수 (WeaponSlots 인덱스 -> 장비 칸 인덱스, 없으면 -1)
	FORCEINLINE int32 GetEquipIndex(const EDC1EquipSlotType InSlotType, const int32 InLocalIndex)
	{
		int32 LocalIndex = 0;
		for (int32 Index = 0; Index < SlotCount; ++Index)
		{
			if (InSlotType != SlotTypes[Index])
			{
				continue;
			}

			if (InLocalIndex == LocalIndex)
			{
				return Index;
			}
			++LocalIndex;
		}
		return -1;
	}
}

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
