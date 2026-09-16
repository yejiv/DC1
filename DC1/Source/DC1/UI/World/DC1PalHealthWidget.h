// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1PalHealthWidget.generated.h"

class UImage;
class UTextBlock;
class UMaterialInterface;
class UWidgetAnimation;
class UDC1StatusBar;
class UDC1DigitCounter;
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

	void RefreshTeamColor();

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

	// 이름
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock>		Text_Name = { nullptr };

	// 속성 아이콘
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage>			Image_Element = { nullptr };

	// 성별 아이콘
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage>			Image_Gender = { nullptr };

	// 레벨
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UDC1DigitCounter>	Counter_Level = { nullptr };

	// 연출.
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation>	Anim_Hit = { nullptr };

private:
	UPROPERTY(EditDefaultsOnly, Category = "UI|Material")
	TSoftObjectPtr<UMaterialInterface>	PartnerBarMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Material")
	TSoftObjectPtr<UMaterialInterface>	WildBarMaterial;

private:
	TWeakObjectPtr<ADC1PalCharacter> OwnerPal = { nullptr };
};
