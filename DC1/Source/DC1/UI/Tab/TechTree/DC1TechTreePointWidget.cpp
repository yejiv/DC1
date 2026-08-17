// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/TechTree/DC1TechTreePointWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"

void UDC1TechTreePointWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (true == ::IsValid(TechPoint_Label) && false == TechPoint_LabelText.IsEmpty())
	{
		TechPoint_Label->SetText(TechPoint_LabelText);
		TechPoint_Label->SetColorAndOpacity(BoxColor);
	}

	if (true == ::IsValid(TechPoint_FrontBar))
	{
		TechPoint_FrontBar->SetColorAndOpacity(BoxColor);
	}

	if (true == ::IsValid(TechPoint_Box))
	{
		TechPoint_Box->SetColorAndOpacity(BoxColor);
	}

	if (true == ::IsValid(TechPoint_Icon))
	{
		TechPoint_Icon->SetBrush(IconBrush);
	}
}

void UDC1TechTreePointWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDC1TechTreePointWidget::SetPoint(int32 NumPoint)
{
	if (false == ::IsValid(TextPoint_Num))
	{
		return;
	}
	
	// Num Tech Point Setting
	TextPoint_Num->SetText(FText::AsNumber(NumPoint));

	EDC1TechPointState CurrentPointState = (0 < NumPoint) ? EDC1TechPointState::HasPoint : EDC1TechPointState::NoPoint;

	if (CurrentPointState == PointState)
	{
		return;
	}

	PointState = CurrentPointState;

	bool HasPoint = PointState == EDC1TechPointState::HasPoint;

	//Overlay Setting
	if (true == ::IsValid(TechPointOverlay))
	{
		float OverlayOpacity = HasPoint ? 1.f : NoPointOverlayOpacity;
		TechPointOverlay->SetRenderOpacity(OverlayOpacity);
	}

	//Box Setting
	if (true == ::IsValid(TechPoint_Box))
	{
		float BoxOpacity = HasPoint ? 1.f : 0.f;
		TechPoint_Box->SetOpacity(BoxOpacity);
	}

	//Background Setting
	if (true == ::IsValid(TechPoint_Background))
	{
		FLinearColor BackgroundColor = HasPoint ? HasPointBackgroundColor : NoPointBackgroundColor;
		TechPoint_Background->SetColorAndOpacity(BackgroundColor);
	}

	//Num Font Setting
	FSlateColor NumFontColor = HasPoint ? HasPointNumColor : NoPointNumColor;
	TextPoint_Num->SetColorAndOpacity(NumFontColor);
}
