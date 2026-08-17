#pragma once

#include "DC1Enums.generated.h"

UENUM(BlueprintType)
enum class EDC1FadeType : uint8
{
	Default = 0 UMETA(DisplayName = "Default"), // 검정 화면
	Spinner = 1 UMETA(DisplayName = "Spinner"), // 검정 화면 + 스프라이트 애니메이션
	Loading = 2 UMETA(DisplayName = "Loading"), // 검정 화면 + 스프라이트 애니메이션 + Tip
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