// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile/DC1Projectile.h"
#include "Data/Pal/DC1PalTypes.h"
#include "DC1PalSphere.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FDC1OnPalCaptured, const FDC1PalInstance&);

/**
 *
 */
UCLASS()
class DC1_API ADC1PalSphere : public ADC1Projectile
{
	GENERATED_BODY()

public:
	ADC1PalSphere();

	// 던진 쪽이 결과를 받으려면 스폰 직후 여기 바인딩한다
	FDC1OnPalCaptured		OnPalCaptured;

protected:
	UFUNCTION()
	void					OnSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	float					ComputeCaptureChance(float InHealthRatio, float InCaptureRate) const;

protected:
	// 스피어 등급. 등급마다 BP를 하나씩 만들고 이 값만 바꿈
	UPROPERTY(EditDefaultsOnly, Category = "Capture", meta = (ClampMin = "0.1"))
	float					CaptureMultiplier = { 1.f };

	// 체력이 확률에 얼마나 기여하는지
	UPROPERTY(EditDefaultsOnly, Category = "Capture", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float					HealthWeight = { 0.8f };
};
