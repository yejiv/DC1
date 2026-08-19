#pragma once

#include "DC1Enums.generated.h"


#pragma region UI ENUM

// FADE TYPE
UENUM(BlueprintType)
enum class EDC1FadeType : uint8
{
	Default = 0 UMETA(DisplayName = "Default"), // 검정 화면
	Spinner = 1 UMETA(DisplayName = "Spinner"), // 검정 화면 + 스프라이트 애니메이션
	Loading = 2 UMETA(DisplayName = "Loading"), // 검정 화면 + 스프라이트 애니메이션 + Tip
};

// TAB BUTTON STATE
UENUM()
enum class EDC1TabButtonState : uint8
{
	Default = 0 UMETA(DisplayName = "Default"),
	Inactive = 1 UMETA(DisplayName = "Inactive"),	// Tab 비활성화
	Active = 2 UMETA(DisplayName = "Active"),		// Tab 활성화
	Disable = 3 UMETA(DisplayName = "Disable"),		// Tab 활성화 불가
};

// TECH SLOT STATE
UENUM()
enum class EDC1TechSlotState : uint8
{
	Default = 0,
	Locked = 1,		// 레벨 부족 || 코스트 부족
	Available = 2,	// 해금 가능 ( 레벨 충족 + 코스트 충분 )
	Unlocked = 3,	// 해금
};

// TECH POINT STATE
UENUM()
enum class EDC1TechPointState : uint8
{
	Default = 0,
	HasPoint = 1,
	NoPoint = 2,
};
#pragma endregion

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
enum class EDC1CollisionLayer : uint8
{
	ECC_Player = ECC_GameTraceChannel1,
	ECC_Pal = ECC_GameTraceChannel2,
	ECC_AllyAttack = ECC_GameTraceChannel3,
	ECC_EnemyAttack = ECC_GameTraceChannel4,
	ECC_Resource = ECC_GameTraceChannel5,
	ECC_ItemInteraction = ECC_GameTraceChannel6,
	ECC_Projectile = ECC_GameTraceChannel7,
};
