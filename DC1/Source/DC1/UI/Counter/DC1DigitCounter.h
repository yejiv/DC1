// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1DigitCounter.generated.h"

class UTextBlock;

/**
 * 유효 자릿수 앞의 0은 어둡게, 값이 0이면 전체를 EmptyColor로 칠한다.
 * 팰스피어 보유량과 무기 잔탄이 같은 위젯을 쓴다.
 */
UCLASS()
class DC1_API UDC1DigitCounter : public UDC1UserWidget
{
	GENERATED_BODY()

private:
	virtual void NativeConstruct() override;

public:
	void SetValue(int32 InValue);

	void SetDigitNumber(int32 InNum);

private:
	// Light 사용 여부
	UPROPERTY(EditDefaultsOnly)
	bool		UseLight = { true };
	// 숫자 자리 수에 따른 Visible 조절
	UPROPERTY(EditAnywhere)
	bool		UseDigitVisible = { false };

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>	Text_Digit100 = { nullptr };
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>	Text_Digit10 = { nullptr };
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>	Text_Digit1 = { nullptr };

	// 999 초과 표기
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>	Text_Plus = { nullptr };

private:
	int32		DigitNumber = { 3 };

private:
	// 불 들어온 자리. 헥스(#B8ECFF)는 sRGB라 리니어로 변환해서 넣는다
	UPROPERTY(EditDefaultsOnly, Category = "Counter")
	FLinearColor	LitColor = { FLinearColor::FromSRGBColor(FColor(0xB8, 0xEC, 0xFF)) };

	// 유효 자릿수 앞의 0
	UPROPERTY(EditDefaultsOnly, Category = "Counter")
	FLinearColor	DimColor = { FLinearColor::FromSRGBColor(FColor(0x7F, 0x86, 0x8C)) };

	// 값이 0일 때
	UPROPERTY(EditDefaultsOnly, Category = "Counter")
	FLinearColor	EmptyColor = { FLinearColor::FromSRGBColor(FColor(0xFF, 0x4D, 0x4D)) };
};