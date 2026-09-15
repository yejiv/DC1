// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1PalHealthWidget.generated.h"

class UTextBlock;
class UDC1StatusBar;
class ADC1PalCharacter;

/**
 * 
 */
UCLASS()
class DC1_API UDC1PalHealthWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void InitForPal(ADC1PalCharacter* InPal); //  팰이 BeginPlay에서 호출

protected:
	virtual void NativeDestruct() override;

private:
	void UnbindPal();

	void HandleHealthChanged(float Current, float Max);

	void RefreshBar(); // 바 갱신 한 곳. 델리게이트와 초기값 읽기가 공유

	void ApplyPalInfo(); // 이름 등 변하지 않는 값. InitForPal에서 1회

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1StatusBar>	HealthBar = { nullptr };

	// 이름을 안 쓰는 WBP도 있을 수 있어 Optional
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock>		Text_Name = { nullptr };

private:
	TWeakObjectPtr<ADC1PalCharacter> OwnerPal = { nullptr };
};
