// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/DC1PalHealthWidget.h"

#include "UI/Bar/DC1StatusBar.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "UI/Counter/DC1DigitCounter.h"

#include "Components/Image.h"
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
	if (false == OwnerPal.IsValid())
	{
		return;
	}

	const UDC1PalData* PalData = OwnerPal->GetPalData();
	if (nullptr == PalData)
	{
		return;
	}

	if (nullptr != Text_Name)
	{
		Text_Name->SetText(PalData->DisplayName);
	}

	if (nullptr != Counter_Level)
	{
		Counter_Level->SetValue(OwnerPal->GetLevel());
	}

	if (nullptr == Image_Element)
	{
		return;
	}

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == ::IsValid(Registry))
	{
		return;
	}

	UTexture2D* ElementIcon = Registry->FindElementIcon(PalData->Element).LoadSynchronous();
	if (nullptr == ElementIcon)
	{
		return;
	}

	Image_Element->SetBrushFromTexture(ElementIcon);
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

	// 퍼센트 계산과 0 나눗셈 방어는 소스가 한다. 위젯은 표시만.
	// 보이고 숨기는 건 팰이 WidgetComponent 를 껐다 켜는 것으로 처리한다
	HealthBar->SetPercent(OwnerPal->GetHealthRatio());
}
