// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/BuildInfo/DC1PalWorkerEntryWidget.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "AbilitySystem/Attribute/DC1PalAttributeSet.h"
#include "AbilitySystemComponent.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UDC1PalWorkerEntryWidget::NativeDestruct()
{
	ClearPal();

	Super::NativeDestruct();
}

void UDC1PalWorkerEntryWidget::SetPal(ADC1PalCharacter* InPal)
{
	ClearPal();

	if (false == ::IsValid(InPal) || false == ::IsValid(PalIcon))
	{
		return;
	}

	Pal = InPal;
	const UDC1PalData* PalData = InPal->GetPalData();

	UMaterialInstanceDynamic* MID = PalIcon->GetDynamicMaterial();
	UTexture2D* IconTexture = (true == ::IsValid(PalData) ? PalData->Icon.LoadSynchronous() : nullptr);

	if (false == ::IsValid(MID) || false == ::IsValid(IconTexture))
	{
		PalIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		MID->SetTextureParameterValue(PalIconImageParamName, IconTexture);
	
		PalIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	UAbilitySystemComponent* ASC = InPal->GetAbilitySystemComponent();
	if (true == ::IsValid(ASC))
	{
		SanityChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UDC1PalAttributeSet::GetSanityAttribute()).AddUObject(this, &ThisClass::OnSanityChanged);
		MaxSanityChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UDC1PalAttributeSet::GetMaxSanityAttribute()).AddUObject(this, &ThisClass::OnSanityChanged);
	}

	RefreshSanity();

	if (true == ::IsValid(StateIcon))
	{
		// TODO Pal State
		StateIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UDC1PalWorkerEntryWidget::ClearPal()
{
	if (true == Pal.IsValid())
	{
		UAbilitySystemComponent* ASC = Pal.Get()->GetAbilitySystemComponent();
		if (true == ::IsValid(ASC))
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UDC1PalAttributeSet::GetSanityAttribute()).Remove(SanityChangedHandle);
			ASC->GetGameplayAttributeValueChangeDelegate(UDC1PalAttributeSet::GetMaxSanityAttribute()).Remove(MaxSanityChangedHandle);

		}
	}

	SanityChangedHandle.Reset();
	MaxSanityChangedHandle.Reset();

	Pal = nullptr;
}

void UDC1PalWorkerEntryWidget::OnSanityChanged(const FOnAttributeChangeData& InData)
{
	RefreshSanity();
}

void UDC1PalWorkerEntryWidget::RefreshSanity()
{
	if (true == Pal.IsValid())
	{
		UAbilitySystemComponent* ASC = Pal.Get()->GetAbilitySystemComponent();
		if (true == ::IsValid(ASC) && true == ::IsValid(SanityBar) && true == ::IsValid(SanityText))
		{
			float Sanity = ASC->GetNumericAttribute(UDC1PalAttributeSet::GetSanityAttribute());
			float MaxSanity = ASC->GetNumericAttribute(UDC1PalAttributeSet::GetMaxSanityAttribute());

			float Percent = MaxSanity > 0.f ? Sanity / MaxSanity : 0.f;
			int32 PercentValue = FMath::RoundToInt(Percent * 100.f);

			SanityBar->SetPercent(Percent);
			SanityText->SetText(FText::AsNumber(PercentValue));
		}
	}
}
