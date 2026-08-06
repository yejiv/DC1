// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Title/DC1TitleButton.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UDC1TitleButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (true == ::IsValid(TextLabel))
	{
		TextLabel->SetText(Button_LabelText);
		NormalColor = TextLabel->GetColorAndOpacity().GetSpecifiedColor();
	}

	if (true == ::IsValid(TitleButton))
	{
		TitleButton->OnClicked.AddDynamic(this, &ThisClass::HandleClicked);
		TitleButton->OnHovered.AddDynamic(this, &ThisClass::HandleHovered);
		TitleButton->OnUnhovered.AddDynamic(this, &ThisClass::HandleUnhovered);
	}

	if (true == ::IsValid(HoverImage))
	{
		HoverMID = HoverImage->GetDynamicMaterial();
		if (true == ::IsValid(HoverMID))
		{
			HoverMID->SetScalarParameterValue("Progress", 0.f);
		}
	}
}

void UDC1TitleButton::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	float Target = IsHovered ? 1.f : 0.f;

	if (true == FMath::IsNearlyEqual(HoverProgress, Target, 0.001f))
	{
		return;
	}

	HoverProgress = FMath::FInterpTo(HoverProgress, Target, InDeltaTime, HoverSpeed);
	
	if (true == FMath::IsNearlyEqual(HoverProgress, Target, 0.001f))
	{
		HoverProgress = Target;
	}

	HoverMID->SetScalarParameterValue("Progress", HoverProgress);
}

void UDC1TitleButton::HandleClicked()
{
	OnClickedDelegate.Broadcast();
}

void UDC1TitleButton::HandleHovered()
{
	IsHovered = true;
	TextLabel->SetColorAndOpacity(HoverColor);
	TextLabel->SetRenderScale(HoverScale);
}

void UDC1TitleButton::HandleUnhovered()
{
	IsHovered = false;
	TextLabel->SetColorAndOpacity(NormalColor);
	TextLabel->SetRenderScale(NormalScale);
}
