#pragma once

#include "GameplayTagContainer.h"
#include "DC1Structs.generated.h"

USTRUCT()
struct FDC1BuildSnapPoint
{
	GENERATED_BODY()
	// 액터 로컬 기준 Snap Point 위치
	FVector WorldLocation = { FVector::ZeroVector };

	// 액터 로컬 기준 Snap Point 법선
	FVector WorldNormal = { FVector::ZeroVector };

	// Snap Point 타입 - 벽, 바닥, 지붕 전용
	FGameplayTag SnapType = { FGameplayTag::EmptyTag };
	// Snap 가능한 상대 타입
	FGameplayTagContainer AcceptTypes;
};