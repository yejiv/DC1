#pragma once

#include "DC1Enums.generated.h"

UENUM(BlueprintType)
enum class EDC1FadeType : uint8
{
	Default = 0 UMETA(DisplayName = "Default"), // 검정 화면
	Spinner = 1 UMETA(DisplayName = "Spinner"), // 검정 화면 + 스프라이트 애니메이션
	Loading = 2 UMETA(DisplayName = "Loading"), // 검정 화면 + 스프라이트 애니메이션 + Tip
};