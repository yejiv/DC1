// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Define/DC1Enums.h"
#include "GameplayTagContainer.h"
#include "DC1PreviewSubsystem.generated.h"

class ADC1Player;
class ADC1PreviewActor;
class ADC1PlayerPreviewActor;
class ADC1PalPreviewActor;
class UDC1EquipComponent;
class UTextureRenderTarget2D;

// 인벤토리 플레이어 미리보기 관리
UCLASS()
class DC1_API UDC1PreviewSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UTextureRenderTarget2D* GetRenderTarget(EDC1PreviewType InPreviewType);

public:
	virtual void Deinitialize() override;

	void BeginPlayerPreview();
	void BeginPalPreview(const FGameplayTag& PalID);
	void EndPreview();

private:
	// 공통: 액터 찾기/스폰 + 상태 기록 (갱신 / 캡처 켜기는 타입별 Begin에서)
	ADC1PreviewActor* BeginPreview(EDC1PreviewType InType);

	ADC1PreviewActor* GetPreviewActor(EDC1PreviewType InType);

private:
	// 렌더 타깃이 없으면 InSize로 생성해서 채우고 반환
	UTextureRenderTarget2D* InitRenderTarget(TObjectPtr<UTextureRenderTarget2D>& InRenderTarget, const FIntPoint& InSize);

	ADC1Player* GetLocalPlayerPawn() const;

	void RefreshPlayerPreview(ADC1Player* InPlayer);
	void RefreshPalPreview(const FGameplayTag& InPalID);

	void HandleWeaponChanged();
	void BindEquipComponent(ADC1Player* InPlayer);
	void UnbindEquipComponent();

private:
	template<typename T>
	T* FindorSpawnPreviewActor(TObjectPtr<T>& InOutActor, TSubclassOf<T> InClass, EDC1PreviewType Type);

private:
	// 맵 아래 먼 곳
	static constexpr float PreviewLocationZ = { -50000.f };

	// Type 별 위치 오프셋
	float PreviewSpacing = { 1000.f };

	UPROPERTY()
	TObjectPtr<ADC1PlayerPreviewActor> PlayerPreviewActor = { nullptr };
	UPROPERTY()
	TObjectPtr<ADC1PalPreviewActor>		PalPreviewActor = { nullptr };

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> PlayerRenderTarget = { nullptr };
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> PalRenderTarget = { nullptr };

	TWeakObjectPtr<UDC1EquipComponent> BoundEquipComponent;

	bool bIsPreviewing = { false };

	// 현재(마지막) 미리보기 타입
	EDC1PreviewType ActiveType = { EDC1PreviewType::Player };
};