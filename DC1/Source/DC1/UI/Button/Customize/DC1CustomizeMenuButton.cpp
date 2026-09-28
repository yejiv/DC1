// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Button/Customize/DC1CustomizeMenuButton.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDC1CustomizeMenuButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (true == ::IsValid(TextLabel))
	{
		TextLabel->SetText(Button_LabelText);
	}

	if (true == ::IsValid(SelectedImage))
	{
		SelectedImage->SetOpacity(0.f);
	}

	if (true == ::IsValid(UserButton))
	{
		const FButtonStyle& Style = UserButton->GetStyle();
		CachedNormalBrush = Style.Normal;
		CachedHoveredBrush = Style.Hovered;
	}

	ApplyState();
}

void UDC1CustomizeMenuButton::HandleClicked()
{
	Super::HandleClicked();		// OnClickedDelegate.Broadcast() - 부모 위젯이 다른 메뉴 버튼들을 Inactive로 돌리는 건 이 델리게이트를 듣고 처리

	SetState(EDC1TabButtonState::Active);
}

void UDC1CustomizeMenuButton::HandleHovered()
{
	// Active(Pressed 고정)/Disable 상태에선 호버로 따로 반응 안 함
	if (ButtonState == EDC1TabButtonState::Active || ButtonState == EDC1TabButtonState::Disable)
	{
		return;
	}

	Super::HandleHovered();

	bIsHovered = true;
}

void UDC1CustomizeMenuButton::HandleUnhovered()
{
	if (ButtonState == EDC1TabButtonState::Active || ButtonState == EDC1TabButtonState::Disable)
	{
		return;
	}

	Super::HandleUnhovered();

	bIsHovered = false;
}

bool UDC1CustomizeMenuButton::SetState(EDC1TabButtonState InState)
{
	if (ButtonState == InState)
	{
		return false;
	}

	ButtonState = InState;
	ApplyState();

	return true;
}

void UDC1CustomizeMenuButton::ApplyState()
{
	const bool bIsActive = (ButtonState == EDC1TabButtonState::Active);

	if (true == ::IsValid(UserButton))
	{
		FButtonStyle Style = UserButton->GetStyle();

		// Active면 Normal/Hovered를 Pressed 브러시로 고정 - 실제 마우스 상태랑 무관하게 눌린 모습 유지
		// Active 해제되면 NativeConstruct에서 캐싱해둔 원래 브러시로 복원
		Style.Normal = bIsActive ? Style.Pressed : CachedNormalBrush;
		Style.Hovered = bIsActive ? Style.Pressed : CachedHoveredBrush;

		UserButton->SetStyle(Style);
	}

	if (true == ::IsValid(SelectedImage))
	{
		SelectedImage->SetOpacity(bIsActive ? 0.2f : 0.f);
	}
}
