// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Craft/DC1CraftSelectWidget.h"

#include "UI/Craft/DC1CraftMaterialEntryWidget.h"

#include "Character/Player/DC1PlayerState.h"

#include "Define/DC1CraftHelper.h"
#include "Define/DC1ItemHelper.h"

#include "Components/Button.h"
#include "Components/DynamicEntryBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "UI/Bar/DC1Slider.h"
#include "Components/TextBlock.h"

void UDC1CraftSelectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (true == ::IsValid(Slider_Count))
	{
		Slider_Count->SetStepSize(1.f);
		Slider_Count->OnValueChanged.AddDynamic(this, &ThisClass::HandleSliderChanged);
	}

	if (true == ::IsValid(Text_Count))
	{
		Text_Count->OnTextCommitted.AddDynamic(this, &ThisClass::HandleCountCommitted);
	}

	if (true == ::IsValid(Button_MinusTen))
	{
		Button_MinusTen->OnClicked.AddDynamic(this, &ThisClass::HandleMinusTenClicked);
	}

	if (true == ::IsValid(Button_MinusOne))
	{
		Button_MinusOne->OnClicked.AddDynamic(this, &ThisClass::HandleMinusOneClicked);
	}

	if (true == ::IsValid(Button_PlusOne))
	{
		Button_PlusOne->OnClicked.AddDynamic(this, &ThisClass::HandlePlusOneClicked);
	}

	if (true == ::IsValid(Button_PlusTen))
	{
		Button_PlusTen->OnClicked.AddDynamic(this, &ThisClass::HandlePlusTenClicked);
	}

	if (true == ::IsValid(Button_Craft))
	{
		Button_Craft->OnClicked.AddDynamic(this, &ThisClass::HandleCraftClicked);
	}
}

void UDC1CraftSelectWidget::SetRecipe(const FDC1ItemRecipeRow& InRecipe, int32 InMaxCraftCount)
{
	CurrentRecipe = InRecipe;
	MaxCraftCount = FMath::Max(InMaxCraftCount, 0);

	const FDC1ItemRow* ProductRow = DC1ItemHelper::FindItemRow(InRecipe.ProductItemID);

	if (true == ::IsValid(Image_ProductIcon))
	{
		if (nullptr != ProductRow && false == ProductRow->ItemIcon.IsNull())
		{
			Image_ProductIcon->SetBrushFromSoftTexture(ProductRow->ItemIcon);
			Image_ProductIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			Image_ProductIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (true == ::IsValid(Text_ProductName))
	{
		const FText ProductName = (nullptr != ProductRow) ? ProductRow->ItemName : FText::GetEmpty();
		const FText ProductCount = FText::AsNumber(InRecipe.ProductCount, &FNumberFormattingOptions::DefaultNoGrouping());

		Text_ProductName->SetText(FText::Format(NSLOCTEXT("Craft", "DetailProductCount", "{0} × {1}"), ProductName, ProductCount));
	}

	const bool CanCraft = (MaxCraftCount > 0);

	if (true == ::IsValid(Slider_Count))
	{
		Slider_Count->SetMinValue(1.f);
		Slider_Count->SetMaxValue(static_cast<float>(FMath::Max(MaxCraftCount, 1)));
		Slider_Count->SetIsEnabled(CanCraft);
		Slider_Count->RefreshFill();
	}

	if (true == ::IsValid(Text_MaxCount))
	{
		Text_MaxCount->SetText(FText::AsNumber(MaxCraftCount, &FNumberFormattingOptions::DefaultNoGrouping()));
	}

	const TArray<UWidget*> CountWidgets = { Text_Count, Button_Craft };
	for (UWidget* CountWidget : CountWidgets)
	{
		if (true == ::IsValid(CountWidget))
		{
			CountWidget->SetIsEnabled(CanCraft);
		}
	}

	CraftCount = CanCraft ? 1 : 0;

	ApplyCountWidgets();
	RefreshMaterials();

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UDC1CraftSelectWidget::ClearRecipe()
{
	CurrentRecipe = FDC1ItemRecipeRow();
	CraftCount = 0;
	MaxCraftCount = 0;

	if (true == ::IsValid(MaterialListBox))
	{
		MaterialListBox->Reset();
	}

	SetVisibility(ESlateVisibility::Hidden);
}

void UDC1CraftSelectWidget::SetCraftCount(int32 InCount)
{
	const int32 NewCraftCount = (MaxCraftCount > 0) ? FMath::Clamp(InCount, 1, MaxCraftCount) : 0;
	const bool IsChanged = (NewCraftCount != CraftCount);

	CraftCount = NewCraftCount;

	ApplyCountWidgets();

	if (true == IsChanged)
	{
		RefreshMaterials();
	}
}

void UDC1CraftSelectWidget::ApplyCountWidgets()
{
	if (true == ::IsValid(Slider_Count))
	{
		Slider_Count->SetValue(static_cast<float>(FMath::Max(CraftCount, 1)));
		Slider_Count->RefreshFill();
	}

	if (true == ::IsValid(Text_CurrentCount))
	{
		Text_CurrentCount->SetText(FText::AsNumber(CraftCount, &FNumberFormattingOptions::DefaultNoGrouping()));
	}

	if (true == ::IsValid(Text_Count))
	{
		Text_Count->SetText(FText::AsNumber(CraftCount, &FNumberFormattingOptions::DefaultNoGrouping()));
	}

	if (true == ::IsValid(RequiredWork_Count))
	{
		const float TotalWork = CurrentRecipe.RequiredWork * FMath::Max(CraftCount, 1);
		RequiredWork_Count->SetText(FText::AsNumber(TotalWork, &FNumberFormattingOptions::DefaultNoGrouping()));
	}

	const bool CanDecrease = (CraftCount > 1);
	const bool CanIncrease = (CraftCount < MaxCraftCount);

	const TArray<UWidget*> DecreaseButtons = { Button_MinusTen, Button_MinusOne };
	for (UWidget* DecreaseButton : DecreaseButtons)
	{
		if (true == ::IsValid(DecreaseButton))
		{
			DecreaseButton->SetIsEnabled(CanDecrease);
		}
	}

	const TArray<UWidget*> IncreaseButtons = { Button_PlusOne, Button_PlusTen };
	for (UWidget* IncreaseButton : IncreaseButtons)
	{
		if (true == ::IsValid(IncreaseButton))
		{
			IncreaseButton->SetIsEnabled(CanIncrease);
		}
	}

	const FLinearColor DecreaseColor = CanDecrease ? ActiveCountButtonColor : InactiveCountButtonColor;
	const FLinearColor IncreaseColor = CanIncrease ? ActiveCountButtonColor : InactiveCountButtonColor;

	const TArray<UImage*> DecreaseImages = { ButtonMinusTen_Image, ButtonMinusOne_Image };
	for (UImage* DecreaseImage : DecreaseImages)
	{
		if (true == ::IsValid(DecreaseImage))
		{
			DecreaseImage->SetColorAndOpacity(DecreaseColor);
		}
	}

	const TArray<UImage*> IncreaseImages = { ButtonPlusOne_Image, ButtonPlusTen_Image };
	for (UImage* IncreaseImage : IncreaseImages)
	{
		if (true == ::IsValid(IncreaseImage))
		{
			IncreaseImage->SetColorAndOpacity(IncreaseColor);
		}
	}
}

void UDC1CraftSelectWidget::RefreshMaterials()
{
	if (true == ::IsValid(MaterialListBox))
	{
		MaterialListBox->Reset();
	}

	const ADC1PlayerState* PlayerState = GetOwningPlayerState<ADC1PlayerState>();
	if (true == CurrentRecipe.RecipeID.IsNone() || false == ::IsValid(PlayerState) || false == ::IsValid(MaterialListBox))
	{
		return;
	}

	const int32 RequiredMultiplier = FMath::Max(CraftCount, 1);

	for (const FDC1RecipeItem& RequiredItem : CurrentRecipe.RequiredItems)
	{
		UDC1CraftMaterialEntryWidget* Entry = MaterialListBox->CreateEntry<UDC1CraftMaterialEntryWidget>();
		if (false == ::IsValid(Entry))
		{
			continue;
		}

		Entry->SetMaterial(RequiredItem.ItemID, DC1CraftHelper::GetCraftItemCount(PlayerState, RequiredItem.ItemID), RequiredItem.Count * RequiredMultiplier);
	}
}

void UDC1CraftSelectWidget::HandleSliderChanged(float InValue)
{
	SetCraftCount(FMath::RoundToInt(InValue));
}

void UDC1CraftSelectWidget::HandleCountCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	if (false == InText.IsNumeric())
	{
		ApplyCountWidgets();
		return;
	}

	SetCraftCount(FCString::Atoi(*InText.ToString()));
}

void UDC1CraftSelectWidget::HandleMinusTenClicked()
{
	SetCraftCount(CraftCount - 10);
}

void UDC1CraftSelectWidget::HandleMinusOneClicked()
{
	SetCraftCount(CraftCount - 1);
}

void UDC1CraftSelectWidget::HandlePlusOneClicked()
{
	SetCraftCount(CraftCount + 1);
}

void UDC1CraftSelectWidget::HandlePlusTenClicked()
{
	SetCraftCount(CraftCount + 10);
}

void UDC1CraftSelectWidget::HandleCraftClicked()
{
	if (true == CurrentRecipe.RecipeID.IsNone() || CraftCount <= 0)
	{
		return;
	}

	OnCraftRequested.Broadcast(CurrentRecipe.RecipeID, CraftCount);
}
