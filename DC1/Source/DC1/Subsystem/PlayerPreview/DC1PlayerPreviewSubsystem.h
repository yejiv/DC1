// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "DC1PlayerPreviewSubsystem.generated.h"

class ADC1Player;
class ADC1PlayerPreviewActor;
class UDC1EquipComponent;
class UTextureRenderTarget2D;

// 인벤토리 플레이어 미리보기 관리
UCLASS()
class DC1_API UDC1PlayerPreviewSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UTextureRenderTarget2D* GetRenderTarget();

public:
	virtual void Deinitialize() override;

	void BeginPreview();
	void EndPreview();

private:
	ADC1PlayerPreviewActor* FindOrSpawnPreviewActor();
	ADC1Player* GetLocalPlayerPawn() const;
	void RefreshPreview();
	void HandleWeaponChanged();
	void BindEquipComponent(ADC1Player* InPlayer);
	void UnbindEquipComponent();

private:
	// 맵 아래 먼 곳
	static constexpr float PreviewLocationZ = { -50000.f };

	UPROPERTY()
	TObjectPtr<ADC1PlayerPreviewActor> PreviewActor = { nullptr };

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> RenderTarget = { nullptr };

	TWeakObjectPtr<UDC1EquipComponent> BoundEquipComponent;

	bool bIsPreviewing = { false };
};
