// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Counter/DC1DigitCounter.h"

#include "Components/TextBlock.h"

void UDC1DigitCounter::NativeConstruct()
{
	Super::NativeConstruct();

	Text_Plus->SetVisibility(ESlateVisibility::Collapsed);
}

void UDC1DigitCounter::SetValue(int32 InValue)
{
	if (InValue >= 1000)
	{
		Text_Plus->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		Text_Plus->SetVisibility(ESlateVisibility::Collapsed);
	}

	TArray<int32> Digits;
	Digits.SetNum(3);
	int32 DigitCount = {};

	for (int32 i = 0; i < 3; ++i)
	{
		if (InValue == 0)
			break;

		Digits[i] = InValue % 10;
		InValue /= 10;
		++DigitCount;
	}

	if (UseDigitVisible)
	{
		SetDigitNumber(FMath::Max(DigitCount, 1));
	}

	Text_Digit1->SetText(FText::AsNumber(Digits[0]));
	Text_Digit10->SetText(FText::AsNumber(Digits[1]));
	Text_Digit100->SetText(FText::AsNumber(Digits[2]));

	FLinearColor Color = DimColor;

	for (int32 i = 2; i >= 0; --i)
	{
		if (Digits[i] > 0)
		{
			Color = UseLight ? LitColor : DimColor;
		}
		else if (i == 0 && Color == DimColor)
		{
			Color = UseLight ? EmptyColor : DimColor;
		}

		switch (i)
		{
		case 0:
			Text_Digit1->SetColorAndOpacity(FSlateColor(Color));
			break;
		case 1:
			Text_Digit10->SetColorAndOpacity(FSlateColor(Color));
			break;
		case 2:
			Text_Digit100->SetColorAndOpacity(FSlateColor(Color));
			break;
		}
	}
}

void UDC1DigitCounter::SetDigitNumber(int32 InNum)
{
	DigitNumber = InNum;
	int32 Digitbit = { 1 };
	Digitbit = Digitbit << DigitNumber;
	Digitbit -= 1;

	Text_Digit1->SetVisibility(Digitbit & 1 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	Text_Digit10->SetVisibility(Digitbit & 2 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	Text_Digit100->SetVisibility(Digitbit & 4 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
