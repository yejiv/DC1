// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Define/DC1Enums.h"
#include "Define/DC1Structs.h"
#include "DC1BuildCheckComponent.generated.h"

struct FDC1BuildSnapPoint;

USTRUCT(BlueprintType)
struct FDC1BuildProbeData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	EDC1BuildProbeDirection ProbeDirection = { EDC1BuildProbeDirection::Down };

	// 상, 하 면에서 X * Y 몇개의 Probe로 검사할건지
	UPROPERTY(EditDefaultsOnly)
	int32 ProbeCountX = { 1 };

	// 상, 하 면에서 X * Y 몇개의 Probe로 검사할건지
	UPROPERTY(EditDefaultsOnly)
	int32 ProbeCountY = { 1 };

	// 건설이 가능한 Probe와 표면사이의 거리
	UPROPERTY(EditDefaultsOnly)
	float MaxTraceDistance = { 10.f };

	// 모서리에서의 Probe 거리 비율. 1.f = 모서리에서 시작, 0.5f = 모서리에서 ( 반지름 * 0.5f ) 만큼 떨어짐
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float ProbeDistanceFromCorner = { 0.95f };

	// 표면의 기울기를 적용할것인가 ( false라면 항상 수직 유지 )
	UPROPERTY(EditDefaultsOnly)
	bool AlignToSurface = { true };

	// 표면의 기울기를 어느정도까지 허용할 것인가
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float MaxSurfaceAngle = { 30.f };
};

USTRUCT(BlueprintType)
struct FDC1BuildSnapData
{
	GENERATED_BODY()

	// 박스 중심에서 면을 향하는 방향, ex) 1,0,0 - 우측면 중앙, 
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	FVector SnapDirection = { FVector::ZeroVector };

	// Snap Point 타입
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag SnapType;

	// Snap 가능한 상대 타입
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer AcceptTypes;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DC1_API UDC1BuildCheckComponent : public UBoxComponent
{
	GENERATED_BODY()

public:	
	UDC1BuildCheckComponent();

public:
	void							SetSnapped(bool InIsSnap) { IsSnap = InIsSnap; }
	const EDC1BuildProbeDirection	GetProbeDirection() const { return BuildProbeData.ProbeDirection; }
	const FVector					GetCheckBoxCenter() const  { return GetComponentLocation(); }
	const float						GetExtentCenterToDirection(const FVector& InWorldDirection, const FQuat& InQuat) const;

public:
	bool							CheckCanBuild();
	const FTransform				ComputePlaceTransform(const FHitResult& InHit, float InYaw) const;
	const FDC1BuildSnapPoint		MakeSideSnapPoint(const FVector& InWorldNormal) const;
	const FDC1BuildSnapPoint		MakeNearestSideSnapPoint(const FVector& InWorldPoint) const;

private:
	bool							CheckSurface();
	bool							CheckOverlap();

	float							ComputeProbeAxisOffset(int32 InIndex, int32 InCount, float InExtent) const;
	FVector							GetAttachLocalOffset() const;
	const FDC1BuildSnapPoint		MakeSideSnapPointFromAxis(bool InIsAxisX, float InAxisSign) const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Build|Check")
	FDC1BuildProbeData			BuildProbeData;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Check")
	bool						DrawDebugProbes = false;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap")
	TArray<FDC1BuildSnapData>	SnapDatas;

private:
	TArray<FDC1BuildSnapPoint>	SnapPoints;
	bool						IsSnap = false;
};
