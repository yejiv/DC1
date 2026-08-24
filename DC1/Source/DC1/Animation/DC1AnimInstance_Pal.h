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
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float				NormalizedSpeed = {};

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	float				BlendInterpSpeed = { 8.f };

	// 제자리 회전을 가상의 이동 속도로 환산하는 계수. (로코모션)
	UPROPERTY(EditDefaultsOnly, Category = "Locomotion", meta = (ClampMin = "0.0"))
	float				TurnSpeedScale = { 0.5f };

private:
	float				MaxLocomotionSpeed = {};
	float				PrevYaw = {};
};
