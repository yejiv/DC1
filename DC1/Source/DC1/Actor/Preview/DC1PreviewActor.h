// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DC1PreviewActor.generated.h"

class UPointLightComponent;
class UPrimitiveComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

// UI 미리보기 공통 (로컬 전용, 전용 조명 + 고정 카메라로 렌더 타깃에 촬영)
UCLASS()
class DC1_API ADC1PreviewActor : public AActor
{
	GENERATED_BODY()

public:
	ADC1PreviewActor();

public:
	void SetRenderTarget(UTextureRenderTarget2D* InRenderTarget);
	virtual void SetCaptureEnabled(bool bInEnabled);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// 복사 메쉬가 전용 조명(채널 2)만 받도록 설정하는 함수
	static void ApplyPreviewLighting(UPrimitiveComponent* InComponent);

	// 캡처가 InPreviewActor의 컴포넌트만 촬영하도록 설정하는 함수
	void SetShowOnlyActor(ADC1PreviewActor* InPreviewActor);

	// Camera 멤버 변수를 바꾸고 바로 반영 (에디터에서 값을 고쳐도 이 값 기준으로 다시 반영됨)
	void SetCameraSetting(float InDistance, float InHeight, float InRight, float InYaw, float InPitch, float InFov);

	// Camera 멤버 변수를 캡처 컴포넌트에 반영
	void ApplyPreviewCameraSetting();

private:
	// 아래 Character / Camera / Light 변수를 컴포넌트에 반영 (컴포넌트 값을 직접 바꾸면 덮어써짐)
	void ApplyPreviewSettings();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USceneComponent> Root = { nullptr };

	// 복사 메쉬의 부모 (복사 메쉬 트랜스폼은 원본 값으로 덮어쓰므로 이걸 옮기고 돌림)
	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USceneComponent> CharacterPivot = { nullptr };

	// 전용 조명 (라이팅 채널 2만 비춤, 월드 조명과 분리)
	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<UPointLightComponent> KeyLight = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<UPointLightComponent> FillLight = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent = { nullptr };

private:
	UPROPERTY(EditAnywhere, Category = "Preview|Character")
	FVector CharacterOffset = { FVector::ZeroVector };

	UPROPERTY(EditAnywhere, Category = "Preview|Character")
	FRotator CharacterRotation = { FRotator::ZeroRotator };

	// 고정 카메라 - 정면(+X)에서 원점을 바라보는 배치를 Yaw만큼 공전 (렌더 타깃 2:3 기준으로 전신 + 든 무기가 들어오게)
	UPROPERTY(EditAnywhere, Category = "Preview|Camera", meta = (ClampMin = "10"))
	float CameraDistance = { 275.f };

	UPROPERTY(EditAnywhere, Category = "Preview|Camera")
	float CameraHeight = { 8.f };

	// 양수면 화면 기준 오른쪽
	UPROPERTY(EditAnywhere, Category = "Preview|Camera")
	float CameraRight = { -30.f };

	UPROPERTY(EditAnywhere, Category = "Preview|Camera")
	float CameraYaw = { -20.f };

	// 양수면 내려다봄
	UPROPERTY(EditAnywhere, Category = "Preview|Camera")
	float CameraPitch = { 3.f };

	UPROPERTY(EditAnywhere, Category = "Preview|Camera", meta = (ClampMin = "5", ClampMax = "170"))
	float CameraFOV = { 30.f };

	UPROPERTY(EditAnywhere, Category = "Preview|Light")
	FVector KeyLightOffset = { 200.f, -120.f, 150.f };

	UPROPERTY(EditAnywhere, Category = "Preview|Light", meta = (ClampMin = "0"))
	float KeyLightIntensity = { 5000.f };

	UPROPERTY(EditAnywhere, Category = "Preview|Light")
	FLinearColor KeyLightColor = { FLinearColor::White };

	UPROPERTY(EditAnywhere, Category = "Preview|Light")
	FVector FillLightOffset = { 200.f, 150.f, 50.f };

	UPROPERTY(EditAnywhere, Category = "Preview|Light", meta = (ClampMin = "0"))
	float FillLightIntensity = { 2000.f };

	UPROPERTY(EditAnywhere, Category = "Preview|Light")
	FLinearColor FillLightColor = { FLinearColor::White };
};
