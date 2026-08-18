#pragma once

#include "DC1Enums.generated.h"

UENUM(BlueprintType)
enum class EDC1FadeType : uint8
{
	Default = 0 UMETA(DisplayName = "Default"), // 검정 화면
	Spinner = 1 UMETA(DisplayName = "Spinner"), // 검정 화면 + 스프라이트 애니메이션
	Loading = 2 UMETA(DisplayName = "Loading"), // 검정 화면 + 스프라이트 애니메이션 + Tip
};


UENUM(BlueprintType)
enum class EDC1ItemType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Weapon = 1 UMETA(DisplayName = "Weapon"),
	Armor = 2 UMETA(DisplayName = "Armor"),
	Material = 3 UMETA(DisplayName = "Material"),
	Consumable = 4 UMETA(DisplayName = "Consumable"),
	Technology = 5 UMETA(DisplayName = "Technology"),
	Food = 6 UMETA(DisplayName = "Food"),
};

UENUM(BlueprintType)
enum class EDC1ItemRarity : uint8
{
	Common = 0 UMETA(DisplayName = "Common"),
	Uncommon = 1 UMETA(DisplayName = "Uncommon"),
	Rare = 2 UMETA(DisplayName = "Rare"),
	Epic = 3 UMETA(DisplayName = "Epic"),
	Legendary = 4 UMETA(DisplayName = "Legendary"),

};

UENUM()
enum class EDC1TechSlotState : uint8
{
	Default = 0,
	Locked = 1,		// 레벨 부족 || 코스트 부족
	Available = 2,	// 해금 가능 ( 레벨 충족 + 코스트 충분 )
	Unlocked = 3,	// 해금
};

UENUM()
enum class EDC1TechPointState : uint8
{
	Default = 0,
	HasPoint = 1,
	NoPoint = 2,	
};

UENUM()
enum class EDC1CollisionLayer : uint8
{
	ECC_Player = ECC_GameTraceChannel1,
	ECC_Pal = ECC_GameTraceChannel2,
	ECC_AllyAttack = ECC_GameTraceChannel3,
	ECC_EnemyAttack = ECC_GameTraceChannel4,
	ECC_Resource = ECC_GameTraceChannel5,
	ECC_ItemInteraction = ECC_GameTraceChannel6,
};
