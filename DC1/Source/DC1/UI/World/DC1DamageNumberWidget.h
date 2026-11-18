// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DC1DamageNumberWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

// 숫자 타입, 색 분기용
UENUM()
enum class EDC1DamageNumberType : uint8
{
	Health,
	Shield,
};

/**
 *
 */
UCLASS()
class DC1_API UDC1DamageNumberWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetDamage(float InAmount, EDC1DamageNumberType InType);

protected:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

private:
	void PlayPop();

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>			Text_Damage = { nullptr };

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation>	Anim_Pop = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "UI|Color")
	FSlateColor						HealthColor;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Color")
	FSlateColor						ShieldColor;

private:
	// 재사용 여부 확인 ( IsShonw == true -> 화면에 있던 Widget )
	bool							IsShown = { false };
};
