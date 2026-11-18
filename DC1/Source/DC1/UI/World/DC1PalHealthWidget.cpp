// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/DC1PalHealthWidget.h"

#include "UI/Bar/DC1StatusBar.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace DC1PalHealth
{
	// 링 머티리얼(M_PalCaptureRing 계열)의 채움 파라미터
	static const FName SanityRingPercentParam = FName("Percent");
}

void UDC1PalHealthWidget::InitForPal(ADC1PalCharacter* InPal)
{
	// 재초기화로 중복 구독되지 않게
	UnbindPal();

	OwnerPal = InPal;
	if (false == OwnerPal.IsValid())
	{
		return;
	}

	ApplyPalInfo();

	BindToPal();
}

void UDC1PalHealthWidget::BindToPal()
{
	if (false == OwnerPal.IsValid())
	{
		return;
	}

	OwnerPal->OnPalHealthChanged.RemoveAll(this);
	OwnerPal->OnPalHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);

	OwnerPal->OnPalSanityChanged.RemoveAll(this);
	OwnerPal->OnPalSanityChanged.AddUObject(this, &ThisClass::HandleSanityChanged);

	RefreshTeamColor();

	RefreshBar();

	RefreshSanity();
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

	if (nullptr != Text_Level)
	{
		Text_Level->SetText(FText::AsNumber(OwnerPal->GetLevel()));
	}

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == ::IsValid(Registry))
	{
		return;
	}

	if (nullptr != Image_ElementIcon00)
	{
		if (UTexture2D* ElementIcon = Registry->FindElementIcon(PalData->Element).LoadSynchronous())
		{
			Image_ElementIcon00->SetBrushFromTexture(ElementIcon);
		}
	}

	if (nullptr != Image_ElementIcon01)
	{
		UTexture2D* SecondIcon = (EDC1PalElement::Neutral == PalData->Element2)
			? nullptr
			: Registry->FindElementIcon(PalData->Element2).LoadSynchronous();

		if (nullptr == SecondIcon)
		{
			Image_ElementIcon01->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			Image_ElementIcon01->SetBrushFromTexture(SecondIcon);
			Image_ElementIcon01->SetVisibility(ESlateVisibility::Visible);
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

void UDC1PalHealthWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToPal();
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
		OwnerPal->OnPalSanityChanged.RemoveAll(this);
	}
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

void UDC1PalHealthWidget::HandleSanityChanged(float Current, float Max)
{
	RefreshSanity();
}

void UDC1PalHealthWidget::RefreshSanity()
{
	if (nullptr == Panel_Sanity || false == OwnerPal.IsValid())
	{
		return;
	}

	// 거점 팰만
	if (nullptr == OwnerPal->GetBasePalBox())
	{
		Panel_Sanity->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	Panel_Sanity->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	const float Sanity = OwnerPal->GetSanity();
	const float MaxSanity = OwnerPal->GetMaxSanity();

	if (nullptr != Text_Sanity)
	{
		Text_Sanity->SetText(FText::AsNumber(FMath::FloorToInt(Sanity)));
	}

	if (nullptr != Image_SanityRing)
	{
		if (UMaterialInstanceDynamic* RingMaterial = Image_SanityRing->GetDynamicMaterial())
		{
			RingMaterial->SetScalarParameterValue(DC1PalHealth::SanityRingPercentParam, (MaxSanity > 0.f) ? (Sanity / MaxSanity) : 0.f);
		}
	}
}
