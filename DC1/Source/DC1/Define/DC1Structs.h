#pragma once

#include "GameplayTagContainer.h"
#include "DC1Structs.generated.h"

class ADC1BuildActor;
class UDC1BuildSocketComponent;

namespace
{
	// 이 안이면 동률로 보고 거리로 가른다
	constexpr float SocketScoreTolerance = 1.f;
}

// 공간 판정용 박스
struct FDC1BuildBox
{
	FTransform Transform = { FTransform::Identity };
	FVector Extent = { FVector::ZeroVector };
};

USTRUCT()
struct FDC1BuildSnapSocket
{
	GENERATED_BODY()

	// Snap Socket 월드 Transform
	// X축 = 상대에 붙을 방향 || 상대를 붙일 방향 / Ctrl 스냅에 사용 O
	// Z축 = Attach 둘레 회전 결정 / Ctrl 스냅 사용 X
	FTransform	SocketTransform = { FTransform::Identity };

	// Snap Socket 타입 - 벽, 바닥, 지붕 전용
	FGameplayTag SnapType = { FGameplayTag::EmptyTag };

	// Snap 가능한 상대 타입
	FGameplayTagContainer AcceptTypes;

	TWeakObjectPtr<const UDC1BuildSocketComponent> SourceSocket;

	// 기준점이 이 소켓 바깥으로 얼마나 벗어났는가
	void EvaluateFrom(const FVector& InRefPoint, float& OutScore, float& OutDistSquared, float& OutFacing) const
	{
		const FVector SocketLocaion = SocketTransform.GetLocation();
		const FVector AttachAxis = SocketTransform.GetUnitAxis(EAxis::X);
		const FVector TangentAxis = SocketTransform.GetUnitAxis(EAxis::Z);

		OutScore = FVector::DotProduct(InRefPoint - SocketLocaion, AttachAxis);
		OutDistSquared = FVector::DistSquared(InRefPoint, SocketLocaion);
		OutFacing = FVector::DotProduct(InRefPoint - SocketLocaion, FVector::CrossProduct(AttachAxis, TangentAxis));
	}

	static bool IsBetter(float InScore, float InDistSquared, float InFacing, float InBestScore, float InBestDistSquared, float InBestFacing)
	{
		// 1순위 - 조준점이 이 면 바깥으로 벗어난 정도
		if (InScore > (InBestScore + SocketScoreTolerance))
		{
			return true;
		}

		if (InScore < (InBestScore - SocketScoreTolerance))
		{
			return false;
		}

		// 2순위 - 소켓 과의 거리
		if (false == FMath::IsNearlyEqual(InDistSquared, InBestDistSquared))
		{
			return InDistSquared < InBestDistSquared;
		}

		// 3순위 - 겹친 소켓 중 어느 면을 바라보는가
		return InFacing > InBestFacing;
	}
};

USTRUCT()
struct FDC1BuildSnapPair
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<const ADC1BuildActor> TargetActor = { nullptr };

	FDC1BuildSnapSocket	TargetSocket;

	FDC1BuildSnapSocket	PreviewSocket;
};