// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1HUDPalIcon.generated.h"

class UImage;
class UTexture2D;
class UMaterialInstanceDynamic;

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
	// 배경은 M_PalIcon의 BackgroundTexture가 그린다. 별도 위젯을 두지 않는다
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>		Image_Icon = {};

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>		Image_Outline = {};

private:
	// M_PalIcon에 만들어 둔 파라미터 이름과 일치해야 한다
	UPROPERTY(EditDefaultsOnly, Category = "Pal Icon")
	FName							IconParameterName = { TEXT("IconTexture") };

	// 빈 칸이면 0. 텍스처 파라미터는 null을 넣어도 머티리얼 기본값으로 돌아가므로 이걸로 끈다
	UPROPERTY(EditDefaultsOnly, Category = "Pal Icon")
	FName							IconOpacityParameterName = { TEXT("IconOpacity") };

	UPROPERTY(EditDefaultsOnly, Category = "Pal Icon")
	FName							BackgroundColorParameterName = { TEXT("BackgroundColor") };

	UPROPERTY(EditDefaultsOnly, Category = "Pal Icon")
	// 곱셈 틴트라서 흰색이 "원본 색 그대로"다
	FLinearColor					Color_Normal = { FLinearColor::White };

	UPROPERTY(EditDefaultsOnly, Category = "Pal Icon")
	FLinearColor					Color_Dead = { FLinearColor(0.8f, 0.1f, 0.1f, 1.f) };

private:
	// Image_Icon의 브러시에 물린 M_PalIcon 인스턴스. 브러시가 텍스처면 null이 된다
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic>	IconMID = {};
};