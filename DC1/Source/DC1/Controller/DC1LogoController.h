// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DC1LogoController.generated.h"

/**
 * 
 */
UCLASS()
class DC1_API ADC1LogoController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:
	void TravelToServer(const FString& IP);
	void TravelToServerFromCachedIP();

private:
	// NumPad9 : 타이틀 UI를 껐다 켜서 뒤 3D 씬(커마 부스 등)을 확인하기 위한 디버그용. 확인 끝나면 통째로 제거
	void OnDebugToggleUI();

private:
	FString CachedIP = {};
};
