// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/World/DC1DamageNumberWidget.h"

#include "Components/TextBlock.h"

void UDC1DamageNumberWidget::SetDamage(float InAmount, EDC1DamageNumberType InType)
{
	if (nullptr == Text_Damage)
	{
		return;
	}

	Text_Damage->SetText(FText::AsNumber(FMath::CeilToInt(InAmount)));
	Text_Damage->SetColorAndOpacity(EDC1DamageNumberType::Shield == InType ? ShieldColor : HealthColor);

	// 이미 화면에 떠 있었으면 재사용 ( 애니메이션 다시 재생 )
	if (true == IsShown)
	{
		PlayPop();
	}
}

void UDC1DamageNumberWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 새로 생성 or 풀에서 꺼낼때
	IsShown = true;
	PlayPop();
}

void UDC1DamageNumberWidget::NativeDestruct()
{
	IsShown = false;

	Super::NativeDestruct();
}

void UDC1DamageNumberWidget::PlayPop()
{
	if (nullptr != Anim_Pop)
	{
		PlayAnimation(Anim_Pop);
	}
}
