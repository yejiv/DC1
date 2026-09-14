// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Define/DC1Enums.h"
#include "Define/DC1Structs.h"
#include "DC1BuildCheckComponent.generated.h"

struct FDC1BuildSnapSocket;

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
struct FDC1BuildSnapSocketData
{
	GENERATED_BODY()

	// 박스 Extent와 곱 - 소켓 위치
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	FVector SnapDirection = { FVector::ZeroVector };

	// Snap Point의 X축
	// 
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	FVector AttachAxis = { FVector::UpVector};

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	FVector TangentAxis = { FVector::ForwardVector};

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

protected:
	virtual void					BeginPlay() override;

public:
	virtual void					TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void							SetCheckSurface(bool InIsCheckSurface) { IsCheckSurface = InIsCheckSurface; }
	const EDC1BuildProbeDirection	GetProbeDirection() const { return BuildProbeData.ProbeDirection; }
	const FVector					GetCheckBoxCenter() const  { return GetComponentLocation(); }
	const bool						GetIsAutoSnap() const { return IsAutoSnap; }
	const float						GetSnappedRotationStep() const { return SnappedRotationStep; }
	const float						GetExtentCenterToDirection(const FVector& InWorldDirection, const FQuat& InQuat) const;

public:
	bool							CheckCanBuild();
	const FTransform				ComputePlaceTransform(const FHitResult& InHit, float InYaw) const;
	bool							GetBuildSocketsWorld(TArray<FDC1BuildSnapSocket>& OutSockets) const;

	float							ComputeOverlapRatio(const FTransform& InOtherTransform, const FVector& InOtherExtent) const;

private:
	bool							CheckSurface();
	bool							CheckOverlap();

	float							ComputeProbeAxisOffset(int32 InIndex, int32 InCount, float InExtent) const;
	FVector							GetAttachLocalOffset() const;
	const FTransform				MakeSnapSocketLocalTransform(const FDC1BuildSnapSocketData& InData) const;

	void							DrawDebugSockets();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Build|Check")
	FDC1BuildProbeData				BuildProbeData;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Check")
	bool							DrawDebugProbes = { false };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap")
	TArray<FDC1BuildSnapSocketData>	SnapDatas;

	UPROPERTY(EditAnywhere, Category = "Build|Snap")
	float							SnappedRotationStep = { 15.f };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap")
	bool							IsAutoSnap = { false };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap")
	float							AutoSnapOverlapAllowableRatio = { 0.1f };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Debug")
	bool							DrawDebugSocket;

private:
	bool							IsCheckSurface = { false };
};
