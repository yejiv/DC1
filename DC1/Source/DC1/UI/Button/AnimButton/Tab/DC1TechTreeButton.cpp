// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Button/AnimButton/Tab/DC1TechTreeButton.h"

#include "Data/DataTable/DC1DataTableTypes.h"

#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Overlay.h"

void UDC1TechTreeButton::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDC1TechTreeButton::NativeDestruct()
{
	if (true == ::IsValid(UnlockedAnim) && true == IsAnimationPlaying(UnlockedAnim))
	{
		SetAnimationCurrentTime(UnlockedAnim, UnlockedAnim->GetEndTime());
		StopAnimation(UnlockedAnim);
	}

	Super::NativeDestruct();
}

void UDC1TechTreeButton::HandleHovered()
{
	if (SlotState == EDC1TechSlotState::Locked)
	{
		return;
	}

	Super::HandleHovered();
	
	if (true == ::IsValid(ButtonImage) && SlotState == EDC1TechSlotState::Available)
	{
		ButtonImage->SetBrush(HoverAvailableButtonImageBrush);
	}
}

void UDC1TechTreeButton::HandleUnhovered()
{
	if (SlotState == EDC1TechSlotState::Locked)
	{
		return;
	}

	Super::HandleUnhovered();

	if (true == ::IsValid(ButtonImage) && SlotState == EDC1TechSlotState::Available)
	{
		ButtonImage->SetBrush(AvailableButtonImageBrush);
	}
}

void UDC1TechTreeButton::InitTechButton(const FDC1TechRow& Row)
{
	TechID = Row.TechID;
	RequiredLevel = Row.RequiredLevel;

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

	// Button Image Setting
	if (true == ::IsValid(ButtonImage))
	{
		FSlateBrush CurrentBrush = IsLocked ? LockedButtonImageBrush : IsUnlocked ? UnlockedButtonImageBrush : AvailableButtonImageBrush;
		ButtonImage->SetBrush(CurrentBrush);
	}

	// Locked Image Setting
	if (true == ::IsValid(LockedImage))
	{
		float LockOverImageOpacity = IsLocked ? LockedImageOpacity : 0.f;

		LockedImage->SetOpacity(LockOverImageOpacity);
	}

	// Available Image Setting
	if (true == ::IsValid(AvailableImage))
	{
		float AvailableOpacity = IsAvailable ? AvailableImageOpacity : 0.f;

		AvailableImage->SetOpacity(AvailableOpacity);
	}

	// Icon Back Image Setting
	if (true == ::IsValid(IconBackImage))
	{
		FLinearColor IconBackImageColor = IsUnlocked ? UnlockedIconBackImageColor : AvailableIconBackImageColor;

		IconBackImage->SetBrushTintColor(IconBackImageColor);
	}

	// TechName Back Image Setting
	if (true == ::IsValid(TechNameBackImage))
	{
		FLinearColor TechNameBackImageColor = IsUnlocked ? UnlockedTechNameBackColor : LockedTechNameBackColor;

		TechNameBackImage->SetColorAndOpacity(TechNameBackImageColor);
	}

	// TechName Text Setting
	if (true == ::IsValid(TechNameText))
	{
		FLinearColor TechNameColor = IsAvailable ? AvailableFontColor : UnlockedFontColor;
		TechNameText->SetColorAndOpacity(TechNameColor);
	}

	// TextLabel ( Need TechPoint ) Setting
	if (true == ::IsValid(TextLabel))
	{
		float CostOpacity = IsAvailable ? 1.f : 0.f;

		TextLabel->SetOpacity(CostOpacity);
	}

	// Overlay Setting
	if (true == ::IsValid(RootOverlay))
	{
		float OverlayOpacity = IsLocked ? LockedOpacity : 1.f;

		RootOverlay->SetRenderOpacity(OverlayOpacity);
	}
}
