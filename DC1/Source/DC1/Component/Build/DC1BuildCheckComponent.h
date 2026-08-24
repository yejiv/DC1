// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Enum/DC1Enums.h"
#include "DC1BuildCheckComponent.generated.h"

USTRUCT(BlueprintType)
struct FDC1BuildProbeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EDC1BuildProbeDirection ProbeDirection = { EDC1BuildProbeDirection::Down };

	// 상, 하 면에서 X * Y 몇개의 Probe로 검사할건지
	UPROPERTY(EditAnywhere)
	int32 ProbeCountX = { 1 };

	// 상, 하 면에서 X * Y 몇개의 Probe로 검사할건지
	UPROPERTY(EditAnywhere)
	int32 ProbeCountY = { 1 };

	// 상, 하 면에서 X * Y 몇개의 Probe로 검사할건지
	UPROPERTY(EditAnywhere)
	float MaxTraceDistance = { 10.f };

	// 모서리에서의 Probe 거리 비율. 1.f = 모서리에서 시작, 0.5f = 모서리에서 ( 반지름 * 0.5f ) 만큼 떨어짐
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float	ProbeDistanceFromCorner = { 0.95f };
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DC1_API UDC1BuildCheckComponent : public UBoxComponent
{
	GENERATED_BODY()

public:	
	UDC1BuildCheckComponent();

public:
	void			SetSnapped(bool InIsSnap) { IsSnap = InIsSnap; }

public:
	bool			CheckCanBuild();

private:
	bool			CheckSurface();
	bool			CheckOverlap();

	float			ComputeProbeAxisOffset(int32 InIndex, int32 InCount, float InExtent) const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "BuildCheck")
	FDC1BuildProbeData	BuildProbeData;

	UPROPERTY(EditDefaultsOnly, Category = "BuildCheck")
	bool				DrawDebugProbes = false;

private:
	bool	IsSnap = false;
};
