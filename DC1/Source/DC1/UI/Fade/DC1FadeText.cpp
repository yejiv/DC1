// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Fade/DC1FadeText.h"
#include "Components/TextBlock.h"

void UDC1FadeText::SetText(const FText& InText)
{
	FadeTip->SetText(InText);
}
