#pragma once

#include "GameplayTagContainer.h"
#include "DC1Structs.generated.h"

USTRUCT()
struct FDC1BuildSnapSocket
{
	GENERATED_BODY()

	// Snap Socket 월드 Transform
	// X축 = AttachAxis ( 상대가 붙어오는 방향 / Ctrl 스냅에 사용 O )
	// Z축 = TangentAxis ( Attach 둘레 회전 결정 / Ctrl 스냅 사용 X )
	FTransform	SocketTransform = { FTransform::Identity };

	// Snap Socket 타입 - 벽, 바닥, 지붕 전용
	FGameplayTag SnapType = { FGameplayTag::EmptyTag };

	// Snap 가능한 상대 타입
	FGameplayTagContainer AcceptTypes;
};