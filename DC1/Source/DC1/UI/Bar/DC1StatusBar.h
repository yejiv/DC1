// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1StatusBar.generated.h"

class UImage;

/**
 * 머티리얼로 그리는 상태 바.
 * UProgressBar로 표현이 안 되는 모양(둥근 캡, 모서리 점, 그라데이션 등)을 위해
 * Brush에 물린 머티리얼의 스칼라 파라미터로 채움을 제어한다.
 *
 * 플레이어 HP, 스태미나, 보스 HP 등 여러 곳에서 재사용한다.
 * 지연 감소를 켜면 값이 줄어들 때 뒤따라오는 두 번째 채움을 같이 갱신하며,
 * 이때 머티리얼에 스칼라 파라미터가 두 개 있어야 한다.
 */
UCLASS()
class DC1_API UDC1StatusBar : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	// 0 ~ 1
	UFUNCTION(BlueprintCallable, Category = "Status Bar")
	void SetPercent(float InPercent);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Status Bar")
	float GetPercent() const { return Percent; }

	// 지연 감소 중이면 뒤바를 즉시 앞바에 맞춘다. 리스폰 등 연출 없이 값을 갱신할 때 쓴다
	UFUNCTION(BlueprintCallable, Category = "Status Bar")
	void SnapToPercent(float InPercent);

	// Material 교체
	UFUNCTION(BlueprintCallable, Category = "Status Bar")
	void SetBarMaterial(UMaterialInterface* InMaterial);

protected:
	// 디자이너 프리뷰와 런타임 초기값을 함께 반영한다
	virtual void NativePreConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void ApplyParameters();

	bool IsCatchingUp() const;

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage> BarImage = { nullptr };

	// 머티리얼에 만들어 둔 스칼라 파라미터 이름과 일치해야 한다
	UPROPERTY(EditAnywhere, Category = "Status Bar")
	FName PercentParameterName = { TEXT("Percent") };

	UPROPERTY(EditAnywhere, Category = "Status Bar", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Percent = { 1.f };

	// 지연 감소 사용 여부. 끄면 뒤바를 항상 앞바와 같게 유지한다
	UPROPERTY(EditAnywhere, Category = "Status Bar|Delayed")
	bool UseDelayedBar = { true };

	UPROPERTY(EditAnywhere, Category = "Status Bar|Delayed", meta = (EditCondition = "UseDelayedBar"))
	FName DelayedPercentParameterName = { TEXT("DelayedPercent") };

	// 값이 줄어든 뒤 따라오기 시작할 때까지의 대기 시간(초)
	UPROPERTY(EditAnywhere, Category = "Status Bar|Delayed", meta = (EditCondition = "UseDelayedBar", ClampMin = "0.0"))
	float CatchUpDelay = { 0.5f };

	// 초당 줄어드는 비율. 0.5면 가득 찬 바가 따라잡는 데 2초
	UPROPERTY(EditAnywhere, Category = "Status Bar|Delayed", meta = (EditCondition = "UseDelayedBar", ClampMin = "0.01"))
	float CatchUpSpeed = { 0.5f };

private:
	float DelayedPercent = { 1.f };
	float RemainingDelay = { 0.f };
};