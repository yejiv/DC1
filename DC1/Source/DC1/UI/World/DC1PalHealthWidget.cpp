// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/DC1PalHealthWidget.h"

#include "UI/Bar/DC1StatusBar.h"
#include "Character/Pal/DC1PalCharacter.h"

void UDC1PalHealthWidget::InitForPal(ADC1PalCharacter* InPal)
{
	// 재초기화로 중복 구독되지 않게
	UnbindPal();

	OwnerPal = InPal;
	if (false == OwnerPal.IsValid())
	{
		return;
	}

	OwnerPal->OnPalHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);

	// 어트리뷰트 델리게이트는 '값이 변할 때만' 오므로 현재 값을 한 번 직접 읽는다.
	// 이게 없으면 다치기 전까지 바가 갱신되지 않는다
	RefreshBar();
}

void UDC1PalHealthWidget::NativeDestruct()
{
	UnbindPal();

	Super::NativeDestruct();
}

void UDC1PalHealthWidget::UnbindPal()
{
	if (true == OwnerPal.IsValid())
	{
		OwnerPal->OnPalHealthChanged.RemoveAll(this);
	}

	OwnerPal.Reset();
}

void UDC1PalHealthWidget::HandleHealthChanged(float Current, float Max)
{
	RefreshBar();
}

void UDC1PalHealthWidget::RefreshBar()
{
	if (false == ::IsValid(HealthBar) || false == OwnerPal.IsValid())
	{
		return;
	}

	const float Ratio = OwnerPal->GetHealthRatio();

	HealthBar->SetPercent(Ratio);

	SetVisibility((Ratio >= 1.f) ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}
