// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile/DC1Projectile.h"
#include "Data/Pal/DC1PalTypes.h"
#include "DC1PalSphere.generated.h"

class UWidgetComponent;
class USoundBase;
class UNiagaraSystem;
class UMaterialInterface;
class ADC1PalCharacter;

DECLARE_MULTICAST_DELEGATE_OneParam(FDC1OnPalCaptured, const FDC1PalInstance&);

// 포획 게이지 표시값. 서버가 정하고 클라는 받아서 그림
USTRUCT()
struct FDC1CaptureDisplay
{
	GENERATED_BODY()

	UPROPERTY()
	int8					Stage = { INDEX_NONE };	//단계

	UPROPERTY()
	float					Chance = { 0.f };	//확률

	UPROPERTY()
	bool					bSuccess = { false };

	// 성공했을 때 잡은 팰. 던진 사람 클라가 포획 알림에 쓴다
	UPROPERTY()
	FDC1PalInstance			CapturedPal;
};

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

	// 서버: 손에서 떼어 조준 방향으로 날린다. 그 전까지는 손 소켓에 붙어 있는 상태 (이동·충돌 꺼짐)
	void					Launch(const FRotator& InAimRotation);

	bool					IsLaunched() const { return bLaunched; }

	FDC1OnPalCaptured		OnPalCaptured;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Open")
	TObjectPtr<UStaticMeshComponent>	TopHalfMesh = { nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Open")
	TObjectPtr<UStaticMeshComponent>	BottomHalfMesh = { nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Capture")
	TObjectPtr<UWidgetComponent>		CaptureWidgetComponent = { nullptr };

protected:
	virtual void			BeginPlay() override;
	// 포획 실패 시 서버가 Destroy -> 클라도 EndPlay에서 실패음 재생
	virtual void			EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 포획 단계음 ( 서버 SetCaptureDisplay / 클라 OnRep 공통 )
	void					PlayCaptureStageSound();

	UFUNCTION()
	void					OnSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	float					ComputeCaptureChance(float InHealthRatio, float InCaptureRate, float InPalLevel) const;	//성공확률

	void					BeginShaking();	//판정 시작
	void					OnShake();		//단계 판정
	void					Resolve(bool bInSuccess);	//판정 후 정리

	void					SetCaptureDisplay(int32 InStage, float InChance, bool bInSuccess);

	UFUNCTION()
	void					OnRep_CaptureDisplay();

	// 던진 사람 화면에만 띄운다
	void					RefreshCaptureWidget();

	// 흡수 연출 ( 팰 발광 + 팰 -> 스피어 트레일, 도착 시 흡수 FX ). 서버 Tick / 클라 OnRep 공통
	void					PlayAbsorbFX();

	UFUNCTION()
	void					OnRep_CapturePal();

	void					SpawnAbsorbFX();

	// 서버: 발광을 보여준 뒤 팰 숨김
	void					HideCapturePal();

	// 스피어 위치에 FX ( 데디 서버 X )
	void					SpawnFX(UNiagaraSystem* InFX) const;

	// 성공을 처음 받았을 때 던진 사람 클라에 포획 알림을 한 번 요청한다
	void					TryRequestCaptureNotice();

protected:
	// 던진 뒤 사라지기까지. 손에 든 동안에는 수명이 흐르지 않는다
	UPROPERTY(EditDefaultsOnly, Category = "Launch", meta = (ClampMin = "0.1"))
	float					LaunchLifeSpan = { 5.f };

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

	// 성공 후 100% 를 보여주고 사라지기까지
	UPROPERTY(EditDefaultsOnly, Category = "Capture", meta = (ClampMin = "0.1"))
	float					SuccessHoldDuration = { 0.8f };

	// 팰 명중
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>	HitPalSound = { nullptr };

	// 팰 외 명중 ( 땅 등 ), 랜덤 재생
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TArray<TObjectPtr<USoundBase>>	HitGroundSounds;

	// 스피어 열림 / 닫힘 ( bSphereOpen 변화를 Tick에서 감지 )
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>	CaptureOpenSound = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>	CaptureCloseSound = { nullptr };

	// 판정 단계마다 ( 1단계부터 )
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>	CaptureShakeSound = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>	CaptureSuccessSound = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>	CaptureFailSound = { nullptr };

	// 1 / 8 팰 발광 오버레이 ( M_Pal_CaptureGlow )
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	TObjectPtr<UMaterialInterface>	PalGlowMaterial = { nullptr };

	// 팰이 빛나는 시간 = 트레일 이동 시간. 끝나면 팰 숨김 + 흡수 FX ( NS_Catch_Trail 의 0.3 과 맞출 것 )
	UPROPERTY(EditDefaultsOnly, Category = "FX", meta = (ClampMin = "0.05"))
	float					PalGlowDuration = { 0.3f };

	// 8 실패 시 팰이 빛나다 원래대로 돌아오기까지
	UPROPERTY(EditDefaultsOnly, Category = "FX", meta = (ClampMin = "0.1"))
	float					FailGlowDuration = { 0.8f };

	// 2 팰 중심 -> 스피어 ( User.TargetPosition )
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	TObjectPtr<UNiagaraSystem>	TrailFX = { nullptr };

	// 3 트레일 도착 ( NS_PalCatch )
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	TObjectPtr<UNiagaraSystem>	AbsorbFX = { nullptr };

	// 4 + 5 닫힘 ( NS_Catch_Close )
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	TObjectPtr<UNiagaraSystem>	CloseFX = { nullptr };

	// 6 판정 단계마다 ( NS_Catch_Shake )
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	TObjectPtr<UNiagaraSystem>	ShakeFX = { nullptr };

	// 7 ( NS_PalCatch_Success )
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	TObjectPtr<UNiagaraSystem>	SuccessFX = { nullptr };

	// 8 ( NS_Catch_Fail )
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	TObjectPtr<UNiagaraSystem>	FailFX = { nullptr };

private:
	bool					bLaunched = { false };	// 손에서 떠났는지 (서버)

	UPROPERTY(ReplicatedUsing = OnRep_CaptureDisplay)
	FDC1CaptureDisplay		CaptureDisplay;

	
	UPROPERTY(Replicated)
	bool					bSphereOpen = { false }; // 열림 여부. 서버가 정함

	// 흡수 중인 팰. 클라 발광 / 트레일 / 실패 발광용 ( TargetPal 은 서버 판정용이라 복제 안 됨 )
	UPROPERTY(ReplicatedUsing = OnRep_CapturePal)
	TObjectPtr<ADC1PalCharacter>	CapturePal = { nullptr };
	float					OpenAlpha = { 0.f };

	// 지난 Tick의 bSphereOpen. 열림/닫힘 사운드 감지용 ( 서버 / 클라 각자 )
	bool					bPrevSphereOpen = { false };

	// 같은 단계 OnRep 중복 재생 방지
	int8					LastSoundStage = { INDEX_NONE };

	FVector					RiseStart = { FVector::ZeroVector };
	FVector					RiseEnd = { FVector::ZeroVector };
	float					RiseAlpha = { 0.f };

	bool					bCaptureStarted = { false };	//재진입 방지

	bool					bCaptureNoticeRequested = { false };

	TWeakObjectPtr<class ADC1PalCharacter>	TargetPal;

	float					CaptureChance = { 0.f };		// 전체 성공 확률. 단계 지수의 밑
	float					ShakeWeightSum = { 0.f };		// ShakeWeights 합. 지수 정규화용
	int32					ShakeIndex = { 0 };				// 지금까지 버틴 단계 수

	FTimerHandle			CaptureTimer;
	FTimerHandle			HidePalTimer;
	FTimerHandle			AbsorbFXTimer;
};
