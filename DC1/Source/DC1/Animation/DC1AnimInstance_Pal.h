// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/DC1AnimInstance.h"
#include "DC1AnimInstance_Pal.generated.h"

UCLASS()
class DC1_API UDC1AnimInstance_Pal : public UDC1AnimInstance
{
	GENERATED_BODY()

public:
	UDC1AnimInstance_Pal(const FObjectInitializer& ObjectInitializer);

public:
	virtual void		NativeInitializeAnimation() override;
	virtual void		NativeUpdateAnimation(float DeltaSeconds) override;

private:
	void				CachePalData();

protected:
	// 블렌드 스페이스 가로축
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float				NormalizedSpeed = {};

	// 블렌드 스페이스 재생 속도
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float				LocomotionPlayRate = { 1.f };

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion", meta = (ClampMin = "1.0"))
	float				MaxPlayRate = { 1.5f };

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	float				BlendInterpSpeed = { 8.f };

	// [제자리 회전]
	// 회전 각속도 -> 이동 속도로 환산하는 계수.
	// 발이 실제로 움직이는 속도 = 각속도(rad/s) x 회전축에서 발까지 거리 이므로, 도/초 기준 계수는 0.017 x 반지름
	UPROPERTY(EditDefaultsOnly, Category = "Locomotion", meta = (ClampMin = "0.0"))
	float				TurnSpeedScale = { 1.2f };

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float				StandingSpeedRatio = { 0.1f };

private:
	float				MaxLocomotionSpeed = {};
	float				WalkLocomotionSpeed = {};
	float				PrevYaw = {};
};
