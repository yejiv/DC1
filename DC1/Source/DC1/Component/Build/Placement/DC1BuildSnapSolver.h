// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Define/DC1Structs.h"
#include "DC1BuildSnapSolver.generated.h"

class ADC1BuildActor;
class UDC1BuildSocketComponent;

// 솔버 한 번 호출에 필요한 입력
// 솔버가 컴포넌트를 역참조하지 않도록 ( 순환 참조 방지 ) 필요한 값만 모아서 넘긴다
struct FDC1BuildSnapInput
{
	ADC1BuildActor*		Preview = { nullptr };

	const APawn*		Pawn = { nullptr };

	// 컨트롤 방향 * PlaceDistance 지점
	FVector				AimPoint = { FVector::ZeroVector };

	// 프리뷰 소켓 선택의 기준 방향
	float				ControlYaw = { 0.f };

	// 회전 입력 누적값
	float				CurrentYaw = { 0.f };

	// 시야 광선 결과 ( 컴포넌트의 TraceBuildActorByView ) - 못 맞혔으면 RayTarget이 nullptr
	ADC1BuildActor*		RayTarget = { nullptr };
	FVector				RayStart = { FVector::ZeroVector };
	FVector				RayEnd = { FVector::ZeroVector };
	FVector				RayImpactPoint = { FVector::ZeroVector };

	// 컴포넌트가 들고 있는 튜닝 값 ( 디테일 패널에서 조정 )
	float				SearchRadius = { 300.f };
	float				AimInflate = { 30.f };
};

UCLASS()
class DC1_API UDC1BuildSnapSolver : public UObject
{
	GENERATED_BODY()

public:
	// 매 틱 - 자동 스냅 자리 찾기 ( 실패하면 컴포넌트가 자유 배치로 간다 )
	bool				FindSnapTransform(const FDC1BuildSnapInput& InInput, FTransform& OutTransform) const;

	// Ctrl 스냅 시작
	bool				BeginManualSnap(const FDC1BuildSnapInput& InInput);

	// 정리 - 조건 걸지 말 것
	void				ClearSnap();

	// 설치 확정 후 기준점 갱신 ( 다음 조각을 방금 놓은 조각에 이어 붙이기 위해 )
	void				OnPlacementConfirmed(const ADC1BuildActor* InPreview);

	const bool			GetIsSnapping() const { return IsSnapping; }
	const float			GetSnapBaseYaw() const { return SnapBaseYaw; }

	// Ctrl 스냅 중 배치점 보정 - 스냅 중이 아니면 InAimPoint를 그대로 돌려준다
	const FVector		GetSnappedPlacePoint(const FVector& InAimPoint) const;

private:
	bool				FindSnapPair(const FDC1BuildSnapInput& InInput, FDC1BuildSnapPair& OutPair) const;

	bool				FindSnapCandidateByRay(const FDC1BuildSnapInput& InInput, TSet<ADC1BuildActor*>& OutCandidates, FVector& OutAimPoint) const;

	void				FindSnapCandidates(const FDC1BuildSnapInput& InInput, const FVector& InAimPoint, TSet<ADC1BuildActor*>& OutCandidates) const;

	void				GatherBlockers(const TSet<ADC1BuildActor*>& InNeighbors, TArray<FDC1BuildBox>& OutBlockers) const;

	bool				FindFreeSnapPair(const FDC1BuildSnapInput& InInput, const TSet<ADC1BuildActor*>& InCandidates, const FVector& InAimPoint,
											const TArray<FDC1BuildSnapSocket>& InPreviewSockets, const TArray<FDC1BuildBox>& InBlockers, FDC1BuildSnapPair& OutPair) const;

	bool				FindBestSnapPair(const FDC1BuildSnapInput& InInput, const TSet<ADC1BuildActor*>& InCandidates, const FVector& InAimPoint,
											const TArray<FDC1BuildSnapSocket>& InPreviewSockets, const TArray<TWeakObjectPtr<const UDC1BuildSocketComponent>>& InExcludedSockets,
											FDC1BuildSnapPair& OutPair) const;

	bool				SelectPreviewSocket(const ADC1BuildActor* InPreview, const FDC1BuildSnapSocket& InTarget, const TArray<FDC1BuildSnapSocket>& InPreviewSockets,
											float InReferenceYaw, FDC1BuildSnapSocket& OutPreview) const;

	const FTransform	MakeSnapTransform(const ADC1BuildActor* InPreview, float InCurrentYaw, const FDC1BuildSnapPair& InPair) const;

private:
	bool									IsSnapping = { false };

	float									SnapBaseYaw = { 0.f };

	FVector									SnapSocketLocationWorld = { FVector::ZeroVector };
	FVector									SnapSocketNormalWorld = { FVector::ZeroVector };

	float									SnapMinDistance = { 0.f };
	float									SnapContactMargin = { 1.f };

	TWeakObjectPtr<const ADC1BuildActor>	SnapTarget;
};
