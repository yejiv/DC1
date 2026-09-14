#pragma once

#include "GameplayTagContainer.h"
#include "DC1Structs.generated.h"

class ADC1BuildActor;

namespace
{
	// 이 안이면 동률로 보고 거리로 가른다
	constexpr float SocketScoreTolerance = 1.f;
}

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

	// 기준점이 이 소켓 바깥으로 얼마나 벗어났는가
	void EvaluateFrom(const FVector& InRefPoint, float& OutScore, float& OutDistSquared) const
	{
		const FVector SocketLocaion = SocketTransform.GetLocation();
		const FVector AttachAxis = SocketTransform.GetUnitAxis(EAxis::X);

		OutScore = FVector::DotProduct(InRefPoint - SocketLocaion, AttachAxis);
		OutDistSquared = FVector::DistSquared(InRefPoint, SocketLocaion);
	}

	static bool IsBetter(float InScore, float InDistSquared, float InBestScore, float InBestDistSquared)
	{
		// 1순위 - 벗어난 정도. 길쭉한 박스에서 끝단면이 잡히는 것을 막는다
		if (InScore > (InBestScore + SocketScoreTolerance))
		{
			return true;
		}

		if (InScore < (InBestScore - SocketScoreTolerance))
		{
			return false;
		}

		// 2순위 - 거리. 윗면 모서리 소켓들처럼 점수가 같을 때 가른다
		return InDistSquared < InBestDistSquared;
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