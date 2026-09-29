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
	Default = 0, 
	Inactive = 1,		// Tab 비활성화
	Active = 2,			// Tab 활성화
	Disable = 3,		// Tab 활성화 불가
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

// PAL SLOT SHIFT
UENUM()
enum class EDC1PalSlotShift : uint8
{
	None = 0,		// 방향 없음. 애니 없이 스냅
	Prev = 1,		// 선택이 이전 인덱스로 (1번 키). 아이콘은 오른쪽으로 밀린다
	Next = 2,		// 선택이 다음 인덱스로 (3번 키). 아이콘은 왼쪽으로 밀린다
};

#pragma endregion

#pragma region BUILD

UENUM(BlueprintType)
enum class EDC1BuildState : uint8
{
	Default = 0 UMETA(DisplayName = "Default"),
	Preview = 1 UMETA(DisplayName = "Preview"),				// 배치 미리보기 ( 로컬 )
	Construction = 2 UMETA(DisplayName = "Construction"),	// 설치 및 건설 진행 중 ( 서버 )
	Completed = 3 UMETA(DisplayName = "Completed"),			// 건설 완료 ( 서버 )
};

UENUM(BlueprintType)
enum class EDC1BuildProbeDirection : uint8
{
	Down = 0 UMETA(DisplayName = "Bottom"),	// 지면 확인
	Up = 1 UMETA(DisplayName = "Up"),		// 천장 확인
};

UENUM(BlueprintType)
enum class EDC1BuildSupportRule : uint8
{
	Ground = 0 UMETA(DisplayName = "Ground"),		// 땅이 지지하는 건축물 ( 땅 위에만 건축 가능한 건축물 ) - 바닥
	Stack = 1 UMETA(DisplayName = "Stack"),			// 소켓 or 하단 Probe로 지지 확인 ( 특정 소켓 또는 지지 받아야만 건축 가능한 건축물 ) - 벽
	RoofSpan = 2 UMETA(DisplayName = "RoofSpan"),	// 소켓으로 붙은 벽의 X,Y 투영 구간에서 N칸 이내 지지 ( 특정 소켓 또는 지지 범위 내에 건축 가능한 건축물 ) - 지붕
	Surface = 3 UMETA(DisplayName = "Surface"),		// 맨땅 or IsBuildableSurafce 위 ( 단순 건축물 )
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

// 인벤토리 슬롯 데이터 출처 (슬롯 위젯이 어느 인벤토리를 표시하는지)
UENUM(BlueprintType)
enum class EDC1InventorySource : uint8
{
	Player = 0 UMETA(DisplayName = "Player"),
	Valuable = 1 UMETA(DisplayName = "Valuable"),
	Chest = 2 UMETA(DisplayName = "Chest"),
};

// 팰 속성
UENUM(BlueprintType)
enum class EDC1PalElement : uint8
{
	Neutral = 0 UMETA(DisplayName = "Neutral"),
	Grass = 1 UMETA(DisplayName = "Grass"),
	Ground = 2 UMETA(DisplayName = "Ground"),
	Electric = 3 UMETA(DisplayName = "Electric"),
	Water = 4 UMETA(DisplayName = "Water"),
	Fire = 5 UMETA(DisplayName = "Fire"),
	Ice = 6 UMETA(DisplayName = "Ice"),
	Dragon = 7 UMETA(DisplayName = "Dragon"),
	Dark = 8 UMETA(DisplayName = "Dark"),

	Max			UMETA(Hidden),
};

// 팰 성별.
UENUM(BlueprintType)
enum class EDC1PalGender : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Male = 1 UMETA(DisplayName = "Male"),
	Female = 2 UMETA(DisplayName = "Female"),

	Max			UMETA(Hidden),
};

UENUM()
enum class EDC1CollisionLayer : uint8
{
	ECC_Player = ECC_GameTraceChannel1,
	ECC_Pal = ECC_GameTraceChannel2,
	ECC_AllyAttack = ECC_GameTraceChannel3,
	ECC_EnemyAttack = ECC_GameTraceChannel4,
	ECC_Resource = ECC_GameTraceChannel5,
	ECC_Interact = ECC_GameTraceChannel6,			// Object, 상호작용 가능여부 판단
	ECC_Projectile = ECC_GameTraceChannel7,
	ECC_BuildSurface = ECC_GameTraceChannel8,		// Trace, 건설 가능여부 판단
	ECC_Building = ECC_GameTraceChannel9,			// Object, Build
};