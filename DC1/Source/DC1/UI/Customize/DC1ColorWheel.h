// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1ColorWheel.generated.h"

class UImage;
class UTexture2D;
class USlider;
class UDC1UserButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDC1OnColorChanged, FLinearColor, InColor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDC1OnColorPickClosed, bool, bConfirmed);

/**
 * Hue/Saturation을 극좌표 휠로, Value(밝기)는 별도 슬라이더로 고르는 위젯 (2D 휠 + 1D Value 바 구성).
 * 휠 그라디언트는 머티리얼이 아니라 런타임에 구운 텍스처(WheelTexture)를 사용한다.
 * 휠 드래그/슬라이더 둘 중 뭐가 바뀌든 Hue/Saturation/Value를 합쳐서 OnColorChanged로 최종 색을 내보낸다.
 * Confirm/Cancel 버튼도 여기 안에 있고, 결과는 OnPickClosed(bConfirmed)로 알려준다 -
 * 팝업을 열고 닫는 건(그리고 Cancel 시 색을 되돌리는 건) Panel의 몫이다.
 */
UCLASS()
class DC1_API UDC1ColorWheel : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	FDC1OnColorChanged OnColorChanged;
	FDC1OnColorPickClosed OnPickClosed;

public:
	// 기존 색상으로 다시 열었을 때 인디케이터/슬라이더 위치를 복원하기 위한 시딩
	void SetSeedColor(const FLinearColor& InColor);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
	void GenerateWheelTexture();

	void UpdateFromLocalPosition(const FVector2D& InLocalPosition, const FVector2D& InLocalSize);

	UFUNCTION()
	void HandleValueChanged(float InValue);

	void HandleConfirmClicked();
	void HandleCancelClicked();

private:
	// Hue/Saturation 기준으로 Indicator를 휠 중심에서 밀어낸 위치로 옮김 (Indicator는 WBP에서 휠 중심에 앵커돼 있어야 함)
	void RepositionIndicator(const FVector2D& InLocalSize);

	void BroadcastColorChanged();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>			WheelImage = { nullptr };

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage>			Indicator = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider>			ValueSlider = { nullptr };

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UDC1UserButton>	ConfirmButton = { nullptr };

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UDC1UserButton>	CancelButton = { nullptr };

	UPROPERTY()
	TObjectPtr<UTexture2D>		WheelTexture = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Customize|ColorWheel")
	int32						WheelResolution = { 256 };

private:
	bool						bIsDragging = { false };

	float						Hue = { 0.f };
	float						Saturation = { 0.f };
	float						Value = { 1.f };
};
