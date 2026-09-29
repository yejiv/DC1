// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Customize/DC1CustomizeWidget.h"

#include "Engine/LocalPlayer.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Appearance/DC1CustomizeViewModel.h"

#include "UI/Button/Customize/DC1CustomizeMenuButton.h"
#include "UI/Button/DC1UserButton.h"
#include "Components/WidgetSwitcher.h"

void UDC1CustomizeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
	SetupMenuButtons();
}

void UDC1CustomizeWidget::NativeDestruct()
{
	for (const TObjectPtr<UDC1CustomizeMenuButton>& Button : MenuButtons)
	{
		if (true == ::IsValid(Button))
		{
			Button->OnClickedDelegate.RemoveAll(this);
		}
	}

	if (true == ::IsValid(StartGameButton))
	{
		StartGameButton->OnClickedDelegate.RemoveAll(this);
	}

	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1CustomizeWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		CustomizeViewModel = VMSubsystem->GetViewModel<UDC1CustomizeViewModel>();
		if (false == CustomizeViewModel.IsValid())
			return;

		CustomizeViewModel->BeginEdit();
	}
}

void UDC1CustomizeWidget::UnbindViewModel()
{
	CustomizeViewModel.Reset();
}

void UDC1CustomizeWidget::SetupMenuButtons()
{
	MenuButtons.Reset();

	if (true == ::IsValid(HeadMenuButton))
	{
		MenuButtons.Add(HeadMenuButton);
	}

	if (true == ::IsValid(HairMenuButton))
	{
		MenuButtons.Add(HairMenuButton);
	}

	if (true == ::IsValid(EyeMenuButton))
	{
		MenuButtons.Add(EyeMenuButton);
	}

	for (const TObjectPtr<UDC1CustomizeMenuButton>& Button : MenuButtons)
	{
		Button->OnClickedDelegate.AddWeakLambda(this, [this, Button]()
			{
				HandleMenuButtonClicked(Button);
			});
	}

	if (true == ::IsValid(StartGameButton))
	{
		StartGameButton->OnClickedDelegate.AddUObject(this, &ThisClass::HandleStartGameClicked);
	}

	HandleMenuButtonClicked(HeadMenuButton);
}

void UDC1CustomizeWidget::HandleMenuButtonClicked(UDC1CustomizeMenuButton* ClickedButton)
{
	for (const TObjectPtr<UDC1CustomizeMenuButton>& Button : MenuButtons)
	{
		if (false == ::IsValid(Button))
		{
			continue;
		}

		Button->SetState(Button == ClickedButton ? EDC1TabButtonState::Active : EDC1TabButtonState::Inactive);
	}

	if (true == ::IsValid(ContentSwitcher) && nullptr != ClickedButton)
	{
		ContentSwitcher->SetActiveWidgetIndex(static_cast<int32>(ClickedButton->GetMenuType()));
	}
}

void UDC1CustomizeWidget::HandleStartGameClicked()
{
	if (false == CustomizeViewModel.IsValid())
	{
		return;
	}

	CustomizeViewModel->ConfirmAppearance();
}
