// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1PalCaptureWidget.generated.h"

class UImage;
class UTextBlock;
class UMaterialInstanceDynamic;

/**
 *
 */
UCLASS()
class DC1_API UDC1PalCaptureWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void			SetCaptureState(int32 InStage, float InChance, bool bInSuccess);

protected:
	virtual void	NativeOnInitialized() override;
	virtual void	NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void			RefreshDisplay();

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>			Image_Ring = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Chance = { nullptr };

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage>			Image_Accent = { nullptr };

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic>	RingMaterial = { nullptr };

private:
	// 버틴 단계 수 순서. 넘치면 마지막 색
	UPROPERTY(EditDefaultsOnly, Category = "Capture|Ring")
	TArray<FLinearColor>		StageColors = { FLinearColor(0.8f, 1.f, 0.f), FLinearColor(0.3f, 1.f, 0.1f), FLinearColor(0.1f, 0.9f, 0.75f) };

	UPROPERTY(EditDefaultsOnly, Category = "Capture|Ring")
	FLinearColor				SuccessColor = { FLinearColor(0.1f, 0.9f, 0.75f) };

	// 한 단계 올라가는 데 걸리는 시간
	UPROPERTY(EditDefaultsOnly, Category = "Capture", meta = (ClampMin = "0.01"))
	float						FillDuration = { 0.4f };

private:
	float						FillFrom = { 0.f };
	float						FillTo = { 0.f };
	float						CurrentFill = { 0.f };

	FLinearColor				ColorFrom = { FLinearColor::White };
	FLinearColor				ColorTo = { FLinearColor::White };
	FLinearColor				CurrentColor = { FLinearColor::White };

	float						FillElapsed = { 0.f };

	bool						bSuccess = { false };
	int32						CurrentStage = { INDEX_NONE };
};
