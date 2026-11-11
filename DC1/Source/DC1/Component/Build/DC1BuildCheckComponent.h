// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Define/DC1Enums.h"
#include "DC1BuildCheckComponent.generated.h"

struct FDC1BuildBox;

class ADC1BuildActor;

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
	float MaxTraceDistance = { 5.f };

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

UCLASS(ClassGroup = (DC1), meta = (BlueprintSpawnableComponent))
class DC1_API UDC1BuildCheckComponent : public UBoxComponent
{
	GENERATED_BODY()

public:	
	UDC1BuildCheckComponent();

public:
	void							SetCheckSurface(bool InIsCheckSurface) { IsCheckSurface = InIsCheckSurface; }
	const EDC1BuildProbeDirection	GetProbeDirection() const { return BuildProbeData.ProbeDirection; }
	const FVector					GetCheckBoxCenter() const  { return GetComponentLocation(); }
	const bool						GetIsAutoSnap() const { return IsAutoSnap; }
	const float						GetSnappedRotationStep() const { return SnappedRotationStep; }
	const bool						GetIsPillar() const { return IsPillar; }

	const float						GetExtentCenterToDirection(const FVector& InWorldDirection, const FQuat& InQuat) const;

public:
	EDC1BuildBlockReason			CheckCanBuild();
	bool							CheckOverlap();

	const FTransform				ComputePlaceTransform(const FHitResult& InHit, float InYaw) const;

	float							ComputeOverlapRatio(const FTransform& InOtherTransform, const FVector& InOtherExtent) const;
	bool							IsBlockedAt(const FTransform& InBoxTransform, const TArray<FDC1BuildBox>& InBlockers) const;

	// 하단 Probe가 맞힌 지지 제공 건축물 수집 ( 해당 건축물 위에 건설 가능한지 && 프리뷰 상태가 아님 )
	// 건축물이 아닌 지면을 맞힌 Probe가 하나라도 존재하면 OutIsOnGround = true
	void							GatherSurfaceSupporters(TArray<ADC1BuildActor*>& OutSupporters, bool& OutIsOnGround) const;

private:
	bool							CheckSurface();

	// LandScape와 겹쳤을때
	bool							IsGroundPenetrated(const UPrimitiveComponent* InGroundComp) const;

	float							ComputeProbeAxisOffset(int32 InIndex, int32 InCount, float InExtent) const;
	FVector							GetAttachLocalOffset() const;				

	// 프로브 격자 루프 / 1개의 결과를 InOnProbe로 넘김
	bool							ForEachProbe(TFunctionRef<bool(const FHitResult& InHit, bool InIsHit)> InOnProbe) const;
	float							ComputeSurfaceAngle(const FHitResult& InHit) const;

	float							GetAllowableGroundDepth() const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Build|Check")
	FDC1BuildProbeData				BuildProbeData;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Check")
	bool							DrawDebugProbes = { false };

	UPROPERTY(EditAnywhere, Category = "Build|Snap")
	float							SnappedRotationStep = { 15.f };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap")
	bool							IsAutoSnap = { false };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Overlap")
	float							SnapOverlapAllowableRatio = { 0.1f };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Overlap")
	float							FreeOverlapAllowableRatio = { 0.005f };

	// true = 기둥 박스 ( Landscape 겹침 허용 + 필수 ) / false = 메인 박스 ( Landscape 겹침 불가 )
	UPROPERTY(EditDefaultsOnly, Category = "Build|Overlap")
	bool							IsPillar = { false };

	// 메인 박스가 Landscape에 ( 박스 높이 * 이 비율 )까지 파묻히는 건 접촉으로 보고 통과 ( 기둥 박스는 사용 안 함 )
	UPROPERTY(EditDefaultsOnly, Category = "Build|Overlap", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float							GroundContactRatio = { 0.05f };

private:
	bool							IsCheckSurface = { false };
};
