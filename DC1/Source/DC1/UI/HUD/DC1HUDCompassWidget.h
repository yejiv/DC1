// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1HUDCompassWidget.generated.h"

class UImage;
class UMaterialInstanceDynamic;

/**
 * HUD 상단 중앙의 방위 나침반.
 * 띠 텍스처를 Wrap으로 감고 UV를 밀어 표시한다. 텍스처 U 1주기 = 360도.
 *
 * Yaw는 매 프레임 바뀌는 연속값이라 ViewModel을 거치지 않고 Tick에서 직접 조회한다.
 */
UCLASS()
class DC1_API UDC1HUDCompassWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	// Brush 머티리얼로부터 MID 생성
	void CreateCompassMID();

	// 화면 중앙이 ViewYaw가 되도록 UV 오프셋을 환산한다
	float CalcUVOffset(float InViewYaw) const;

private:
	void ApplyToMaterial();

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>						Image_Compass = { nullptr };

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic>	CompassMID = { nullptr };

private:
	// 머티리얼의 UV 오프셋 스칼라 파라미터 이름
	UPROPERTY(EditDefaultsOnly, Category = "Compass")
	FName	OffsetParamName = { TEXT("Offset") };

	UPROPERTY(EditDefaultsOnly, Category = "Compass")
	FName	SpanParamName = { TEXT("UVScale") };

	// 위젯 폭에 담을 각도. 나침반 마커를 붙일 때도 같은 값을 써야 한다
	UPROPERTY(EditDefaultsOnly, Category = "Compass", meta = (ClampMin = "10.0", ClampMax = "360.0"))
	float	SpanDegrees = { 120.f };

private:

	float		UVOffset = {};
};
