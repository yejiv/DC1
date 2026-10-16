// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DC1PlayerPreviewActor.generated.h"

class ADC1Player;
class UMeshComponent;
class UPointLightComponent;
class USceneCaptureComponent2D;
class USkeletalMeshComponent;
class UTextureRenderTarget2D;

// 인벤토리 플레이어 미리보기 (로컬 전용, 플레이어를 복사한 메쉬를 전용 조명으로 촬영)
UCLASS()
class DC1_API ADC1PlayerPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	ADC1PlayerPreviewActor();

public:
	void SetRenderTarget(UTextureRenderTarget2D* InRenderTarget);
	void CopyFromPlayer(ADC1Player* InPlayer);
	void CopyAttachments(ADC1Player* InPlayer);
	void SetCaptureEnabled(bool bInEnabled);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	// 아래 Character / Light 변수를 컴포넌트에 반영 (컴포넌트 값을 직접 바꾸면 덮어써짐)
	void ApplyPreviewSettings();
	void CopySkeletalMesh(USkeletalMeshComponent* InSource, USkeletalMeshComponent* InCopy);
	void CopyAttachedMesh(UMeshComponent* InSource, USceneComponent* InSourceParent, USkeletalMeshComponent* InCopyParent, FName InSocketName);
	USkeletalMeshComponent* FindCopyParent(const USceneComponent* InSourceParent) const;
	void ClearAttachments();

	// 복사 메쉬가 전용 조명(채널 2)만 받도록 설정하는 함수
	static void ApplyPreviewLighting(UPrimitiveComponent* InComponent);

private:
	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USceneComponent> Root = { nullptr };

	// 플레이어 메쉬 복사본 (Body / Head / Hair / HairEquip)
	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USkeletalMeshComponent> CopyBody = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USkeletalMeshComponent> CopyHead = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USkeletalMeshComponent> CopyHair = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USkeletalMeshComponent> CopyHairEquip = { nullptr };

	// 전용 조명 (라이팅 채널 2만 비춤, 월드 조명과 분리)
	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<UPointLightComponent> KeyLight = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<UPointLightComponent> FillLight = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent = { nullptr };

	// CopyBody의 부모 (CopyBody 트랜스폼은 플레이어 값으로 덮어쓰므로 이걸 옮기고 돌림)
	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USceneComponent> CharacterPivot = { nullptr };

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

	// 원본 메쉬 -> 복사 메쉬 매핑 (부착 메쉬의 부모 찾기용, CopyFromPlayer마다 초기화)
	TMap<const USceneComponent*, TObjectPtr<USkeletalMeshComponent>> SourceToCopy;

	// 런타임에 복제한 부착 메쉬 (무기/도구 등)
	UPROPERTY()
	TArray<TObjectPtr<UMeshComponent>> AttachmentCopies;
};
