#include "UI/Tab/Inventory/Status/DC1MenuGaugeWidget.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

namespace
{
	FText FormatGaugeNumber(float InValue)
	{
		FNumberFormattingOptions Options;
		Options.UseGrouping = false;
		Options.MaximumFractionalDigits = 0;
		return FText::AsNumber(InValue, &Options);
	}
}

void UDC1MenuGaugeWidget::SetValues(float InCurrent, float InMax)
{
	if (::IsValid(ProgressBar_Gauge))
	{
		ProgressBar_Gauge->SetPercent(0.f < InMax ? FMath::Clamp(InCurrent / InMax, 0.f, 1.f) : 0.f);
	}

	if (::IsValid(TextBlock_Current))
	{
		TextBlock_Current->SetText(FormatGaugeNumber(InCurrent));
	}

	if (::IsValid(TextBlock_Max))
	{
		TextBlock_Max->SetText(FText::Format(INVTEXT("/{0}"), FormatGaugeNumber(InMax)));
	}
}

void UDC1MenuGaugeWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// 디자이너에서도 인스턴스별 설정이 보이도록 여기서 적용
	if (::IsValid(ProgressBar_Gauge))
	{
		ProgressBar_Gauge->SetFillColorAndOpacity(FillColor);
	}

	if (::IsValid(Image_Line))
	{
		Image_Line->SetColorAndOpacity(FLinearColor(FillColor.R, FillColor.G, FillColor.B, FillColor.A * 0.5f));
	}

	if (::IsValid(Image_Icon))
	{
		Image_Icon->SetBrushFromTexture(GaugeIcon);
		Image_Icon->SetVisibility(nullptr != GaugeIcon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (::IsValid(TextBlock_Title))
	{
		TextBlock_Title->SetText(GaugeTitle);
	}
}
