// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/DC1PalHealthWidget.h"

#include "UI/Bar/DC1StatusBar.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalData.h"

#include "Components/TextBlock.h"

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

	ApplyPalInfo();

	RefreshBar();
}

void UDC1PalHealthWidget::ApplyPalInfo()
{
	if (nullptr == Text_Name || false == OwnerPal.IsValid())
	{
		return;
	}

	const UDC1PalData* PalData = OwnerPal->GetPalData();
	if (nullptr == PalData)
	{
		return;
	}

	Text_Name->SetText(PalData->DisplayName);
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

	HealthBar->SetVisibility((Ratio >= 1.f) ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}
