// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/World/DC1DamageNumberWidget.h"
#include "DC1DamageNumberComponent.generated.h"

class UWidgetComponent;
struct FGameplayEffectModCallbackData;

USTRUCT()
struct FDC1DamageNumberEntry
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWidgetComponent>	Number = { nullptr };
	// 만료 시간
	double							ExpireTime = { 0.0 };
};

/**
 *
 */
UCLASS()
class DC1_API UDC1DamageNumberComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDC1DamageNumberComponent();

	// 서버
	static void NotifyDamage(const FGameplayEffectModCallbackData& InData, float InAmount, EDC1DamageNumberType InType);

	void ShowDamageNumber(const FVector& InLocation, float InAmount, EDC1DamageNumberType InType);

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// 팰 작업 데미지 표시 여부(거리)
	static bool IsWorkNumberHidden(const FGameplayEffectModCallbackData& InData, const AActor* InTarget);
	// 데미지(WidgetComp) 위치 계산 
	static FVector GetNumberLocation(const AActor* InTarget);
	// 모든 데미지 표시 여부(거리)
	bool IsTooFarFromCamera(const FVector& InLocation) const;
	// 풀 확인 ( Free -> (Activate(상한)) -> New 순서)
	UWidgetComponent* AcquireNumber();

	UWidgetComponent* CreateNumber();

	void ReleaseNumber(UWidgetComponent* InNumber);

	AActor* GetOrSpawnHost();

private:
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber")
	TSubclassOf<UDC1DamageNumberWidget>		WidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber", meta = (ClampMin = "0.1"))
	float									NumberLifetime = { 1.f };
	// 풀 최대 개수
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber", meta = (ClampMin = "1"))
	int32									MaxNumbers = { 30 };
	// 카메라 기준 데미지 표시 최대 거리
	UPROPERTY(EditDefaultsOnly, Category = "DamageNumber", meta = (ClampMin = "100"))
	float									MaxShowDistance = { 5000.f };

	// 활성 풀
	UPROPERTY(Transient)
	TArray<FDC1DamageNumberEntry>			ActiveNumbers;
	// 대기 풀
	UPROPERTY(Transient)
	TArray<TObjectPtr<UWidgetComponent>>	FreeNumbers;

	// 숫자 WidgetComponent Owner ( DamageComp가 PC 소유라 기본 Hidden -> 비 Hiddne인 Actor Spawn 후 관리 )
	UPROPERTY(Transient)
	TObjectPtr<AActor>						NumberHost = { nullptr };
};
