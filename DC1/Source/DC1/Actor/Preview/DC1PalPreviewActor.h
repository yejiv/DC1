// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/Preview/DC1PreviewActor.h"
#include "DC1PalPreviewActor.generated.h"

class USkeletalMeshComponent;
struct FDC1PalPreviewSettings;

// 팰 미리보기 (로컬 전용, PalData의 미리보기 설정으로 메쉬만 교체해서 촬영)
UCLASS()
class DC1_API ADC1PalPreviewActor : public ADC1PreviewActor
{
	GENERATED_BODY()

public:
	ADC1PalPreviewActor();

public:
	// 메쉬 / Idle 애니 / 카메라 교체
	void SetPalPreview(const FDC1PalPreviewSettings& InSettings);

	// 미리보기가 꺼져 있는 동안 애니 Tick 정지
	void SetAnimationEnabled(bool bInEnabled);

private:
	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USkeletalMeshComponent> PalMesh = { nullptr };
};
