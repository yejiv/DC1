// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Button/AnimButton/Slot/DC1TechTreeButton.h"

#include "Data/DataTable/DC1DataTableTypes.h"

#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Overlay.h"


void UDC1TechTreeButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (SlotState == EDC1TechSlotState::Locked)
	{
		return;
	}

	if (true == ::IsValid(ButtonImage) && SlotState == EDC1TechSlotState::Available)
	{
		ButtonImage->SetBrush(AvailableButtonImageBrush);
	}
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

void UDC1TechTreeButton::InitTechButton(const FDC1TechRow& InRow)
{
	TechTag = InRow.TechTag;
	RequiredLevel = InRow.RequiredLevel;

	if (true == ::IsValid(TechNameText))
	{
		TechNameText->SetText(InRow.DisplayName);
	}

	if (true == ::IsValid(CategoryNameText))
	{
		CategoryNameText->SetText(InRow.CategoryName);
	}

	if (true== ::IsValid(TextLabel))
	{
		TextLabel->SetText(FText::AsNumber(InRow.UnlockCost));
	}

	if (true == ::IsValid(TechIconImage) && false == InRow.Icon.IsNull())
	{
		TechIconImage->SetBrushFromSoftTexture(InRow.Icon);
	}
}

void UDC1TechTreeButton::SetState(EDC1TechSlotState InNewState)
{
	if (SlotState == InNewState)
	{
		return;
	}

	// 처음 초기화 시점 Unlocked Anim 스킵
	if (SlotState != EDC1TechSlotState::Default && InNewState == EDC1TechSlotState::Unlocked)
	{
		FDC1AnimPlayInfo PlayInfo;

		// 일회성 해금 연출 - 끝나면 원복
		PlayInfo.PlayMode = EUMGSequencePlayMode::Forward;
		PlayInfo.RestoreState = true;

		PlayAnim(UnlockedAnim, PlayInfo);
	}

	SlotState = InNewState;

	ApplyState();
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
