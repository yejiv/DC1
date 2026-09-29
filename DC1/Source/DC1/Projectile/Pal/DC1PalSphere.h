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

	virtual void			Tick(float DeltaSeconds) override;

	virtual void			GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FDC1OnPalCaptured		OnPalCaptured;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Open")
	TObjectPtr<UStaticMeshComponent>	TopHalfMesh = { nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Open")
	TObjectPtr<UStaticMeshComponent>	BottomHalfMesh = { nullptr };

protected:
	UFUNCTION()
	void					OnSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	float					ComputeCaptureChance(float InHealthRatio, float InCaptureRate, float InPalLevel) const;	//성공확률

	void					BeginShaking();	//판정 시작
	void					OnShake(); //단계 판정
	void					Resolve(bool bInSuccess);	//판정 후 정리

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Capture", meta = (ClampMin = "1.0"))
	float					SphereLevel = { 7.f };

	// 단계별 난이도 비중. 합으로 정규화해서 각 단계의 지수로 씀
	UPROPERTY(EditDefaultsOnly, Category = "Capture")
	TArray<float>			ShakeWeights = { 4.f, 3.f, 2.f };

	// 단계 간격. SetTimer가 0 이하를 해제로 처리해 0 불가
	UPROPERTY(EditDefaultsOnly, Category = "Capture", meta = (ClampMin = "0.1"))
	float					ShakeInterval = { 0.8f };

	// 명중부터 첫 단계까지.
	UPROPERTY(EditDefaultsOnly, Category = "Capture", meta = (ClampMin = "0.1"))
	float					AbsorbDuration = { 0.6f };

	// 스피어 열리는 거리
	UPROPERTY(EditDefaultsOnly, Category = "Open", meta = (ClampMin = "0.0"))
	float					OpenDistance = { 20.f };

	// 속도
	UPROPERTY(EditDefaultsOnly, Category = "Open", meta = (ClampMin = "0.1"))
	float					OpenSpeed = { 6.f }; //초당 진행 비율

	UPROPERTY(EditDefaultsOnly, Category = "Open")
	FVector					RiseOffset = { 0.f, 60.f, 100.f };

	UPROPERTY(EditDefaultsOnly, Category = "Open", meta = (ClampMin = "0.05"))
	float					RiseDuration = { 0.25f };

private:
	
	UPROPERTY(Replicated)
	bool					bSphereOpen = { false }; // 열림 여부. 서버가 정함
	float					OpenAlpha = { 0.f };

	FVector					RiseStart = { FVector::ZeroVector };
	FVector					RiseEnd = { FVector::ZeroVector };
	float					RiseAlpha = { 0.f };

	bool					bCaptureStarted = { false };	//재진입 방지

	TWeakObjectPtr<class ADC1PalCharacter>	TargetPal;

	float					CaptureChance = { 0.f };		// 전체 성공 확률. 단계 지수의 밑
	float					ShakeWeightSum = { 0.f };		// ShakeWeights 합. 지수 정규화용
	int32					ShakeIndex = { 0 };				// 지금까지 버틴 단계 수

	FTimerHandle			CaptureTimer;
};
