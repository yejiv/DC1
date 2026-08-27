#include "UI/Item/Drag/DC1ItemDragWidget.h"

#include "Data/DataTable/DC1DataTableTypes.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

UDC1ItemDragWidget::UDC1ItemDragWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetVisibility(ESlateVisibility::Visible);
}

void UDC1ItemDragWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(SizeBox_Root))
	{
		SizeBox_Root->SetWidthOverride(100);
		SizeBox_Root->SetHeightOverride(100);
	}
}

void UDC1ItemDragWidget::InitDragWidget(const FDC1ItemRow* InItemData, int32 InStackCount)
{
	if (!InItemData)
	{
		return;
	}

	if (IsValid(Image_Icon))
	{
		if (InItemData->ItemIcon.IsPending())
		{
			Image_Icon->SetBrushFromTexture(nullptr);
		}
		else
		{
			Image_Icon->SetBrushFromTexture(InItemData->ItemIcon.Get());
		}
		Image_Icon->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.8f));
	}

	if (IsValid(Text_Count))
	{
		if (InStackCount > 1)
		{
			Text_Count->SetText(FText::AsNumber(InStackCount));
			Text_Count->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			Text_Count->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
