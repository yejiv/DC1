// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1HUDPalIcon.generated.h"

class UImage;
class UTexture2D;

/**
 * HUD 팰 캐러셀의 한 칸. 아이콘 + 상태 연출(사망 시 붉은 배경, 소환 중 아웃라인).
 * VM을 모르고, HUDPalWidget이 오프셋으로 꺼낸 값을 넣어준다
 */
UCLASS()
class DC1_API UDC1HUDPalIcon : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

public:
	// null이면 빈 칸 기본 텍스처
	void SetIcon(UTexture2D* InIcon);

	// 뒷배경 붉게
	void SetDead(bool bInDead);

	// 아웃라인 표시
	void SetDeployed(bool bInDeployed);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>		Image_Background = {};

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>		Image_Icon = {};

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>		Image_Outline = {};

private:
	FLinearColor					Color_Red = { FLinearColor(0.8f, 0.1f, 0.1f, 1.f) };
};