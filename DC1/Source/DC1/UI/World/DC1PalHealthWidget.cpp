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
#include "Materials/MaterialInterface.h"

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

	RefreshTeamColor();

	RefreshBar();
}

void UDC1PalHealthWidget::RefreshTeamColor()
{
	if (false == ::IsValid(HealthBar) || false == OwnerPal.IsValid())
	{
		return;
	}

	const bool IsPartner = (EDC1Team::Player == OwnerPal->GetTeam());

	const TSoftObjectPtr<UMaterialInterface>& BarMaterial = IsPartner ? PartnerBarMaterial : WildBarMaterial;

	if (true == BarMaterial.IsNull())
	{
		return;
	}

	HealthBar->SetBarMaterial(BarMaterial.LoadSynchronous());
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

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == ::IsValid(Registry))
	{
		return;
	}

	if (nullptr != Image_Element)
	{
		if (UTexture2D* ElementIcon = Registry->FindElementIcon(PalData->Element).LoadSynchronous())
		{
			Image_Element->SetBrushFromTexture(ElementIcon);
		}
	}

	if (nullptr != Image_Gender)
	{
		UTexture2D* GenderIcon = Registry->FindGenderIcon(OwnerPal->GetGender()).LoadSynchronous();
		if (nullptr == GenderIcon)
		{
			Image_Gender->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			Image_Gender->SetBrushFromTexture(GenderIcon);
			Image_Gender->SetVisibility(ESlateVisibility::Visible);
		}
	}
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
	const bool IsDamaged = ::IsValid(HealthBar) && OwnerPal.IsValid()
		&& (OwnerPal->GetHealthRatio() < HealthBar->GetPercent());

	RefreshBar();

	if (true == IsDamaged && nullptr != Anim_Hit)
	{
		PlayAnimation(Anim_Hit);
	}
}

void UDC1PalHealthWidget::RefreshBar()
{
	if (false == ::IsValid(HealthBar) || false == OwnerPal.IsValid())
	{
		return;
	}

	HealthBar->SetPercent(OwnerPal->GetHealthRatio());
}
