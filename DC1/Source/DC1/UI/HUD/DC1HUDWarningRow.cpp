#include "UI/HUD/DC1HUDWarningRow.h"

#include "Components/TextBlock.h"
#include "Components/Border.h"

void UDC1HUDWarningRow::SetWarning(const FText& InText, const FLinearColor& InColor)
{
	if (true == ::IsValid(Text_Warning))
	{
		Text_Warning->SetText(InText);
	}
	
	if (true == ::IsValid(WarningImageBox))
	{
		WarningImageBox->SetContentColorAndOpacity(InColor);
	}
}
