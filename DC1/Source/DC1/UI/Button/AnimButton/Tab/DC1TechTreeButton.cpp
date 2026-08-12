// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Button/AnimButton/Tab/DC1TechTreeButton.h"

#include "Data/DataTable/DC1DataTableTypes.h"

#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UDC1TechTreeButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDC1TechTreeButton::HandleHovered()
{
	Super::HandleHovered();
	
	if (true == ::IsValid(ButtonHoverImage) && SlotState == EDC1TechSlotState::Available)
	{
		ButtonHoverImage->SetOpacity(1.f);
	}
}

void UDC1TechTreeButton::HandleUnhovered()
{
	Super::HandleUnhovered();

	if (true == ::IsValid(ButtonHoverImage) && SlotState == EDC1TechSlotState::Available)
	{
		ButtonHoverImage->SetOpacity(0.f);
	}
}

void UDC1TechTreeButton::InitTechButton(const FDC1TechRow& Row)
{
	TechID = Row.TechID;

	if (true == ::IsValid(TechNameText))
	{
		TechNameText->SetText(Row.DisplayName);
	}

	if (true == ::IsValid(CategoryNameText))
	{
		CategoryNameText->SetText(Row.CategoryName);
	}

	if (true== ::IsValid(TextLabel))
	{
		TextLabel->SetText(FText::AsNumber(Row.UnlockCost));
	}

	if (true == ::IsValid(TechIconImage) && true == ::IsValid(Row.Icon))
	{
		TechIconImage->SetBrushFromTexture(Row.Icon);
	}
}

void UDC1TechTreeButton::SetState(EDC1TechSlotState NewState)
{
	if (SlotState == NewState)
	{
		return;
	}

	SlotState = NewState;

	ApplyState();

	if (SlotState == EDC1TechSlotState::Unlocked && true == ::IsValid(UnlockedAnim))
	{
		PlayAnimation(UnlockedAnim, 0.f, 1);
	}
}

void UDC1TechTreeButton::ApplyState()
{
	if (SlotState == EDC1TechSlotState::Default)
	{
		return;
	}

	bool IsLocked = (SlotState == EDC1TechSlotState::Locked);
	bool IsAvailable = (SlotState == EDC1TechSlotState::Available);
	bool IsUnlocked = (SlotState == EDC1TechSlotState::Unlocked);
	
	if (false == IsAvailable && true == ::IsValid(ButtonHoverImage))
	{
		ButtonHoverImage->SetOpacity(0.f);
	}

	if (true == ::IsValid(LockedImage))
	{
		float LockedOpacity = IsLocked ? LockedImageOpacity : 0.f;

		LockedImage->SetOpacity(LockedOpacity);
	}

	if (true == ::IsValid(AvailableImage))
	{
		float AvailableOpacity = IsAvailable ? AvailableImageOpacity : 0.f;

		AvailableImage->SetOpacity(AvailableOpacity);
	}

	if (true == ::IsValid(IconBackImage))
	{
		FLinearColor IconBackImageColor = IsUnlocked ? UnlockedIconBackImageColor : AvailableIconBackImageColor;

		IconBackImage->SetBrushTintColor(IconBackImageColor);
	}

	if (true == ::IsValid(TechNameBackImage))
	{
		FLinearColor TechNameBackImageColor = IsUnlocked ? UnlockedTechNameBackColor : LockedTechNameBackColor;

		TechNameBackImage->SetColorAndOpacity(TechNameBackImageColor);
	}

	if (true == ::IsValid(TechNameText))
	{
		FLinearColor TechNameColor = IsAvailable ? AvailableFontColor : UnlockedFontColor;
		TechNameText->SetColorAndOpacity(TechNameColor);
	}

	if (true == ::IsValid(TextLabel))
	{
		float CostOpacity = IsAvailable ? 1.f : 0.f;

		TextLabel->SetOpacity(CostOpacity);
	}
}
