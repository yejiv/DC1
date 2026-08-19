// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/DC1TabWidget.h"

#include "UI/Button/AnimButton/Tab/DC1TabHeaderButton.h"

#include "Subsystem/DC1UIManagerSubsystem.h"

void UDC1TabWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	for (const TPair<FGameplayTag, int32>& Pair : TabToSwitchIndex)
	{
		SwitchIndexToTab.Add(Pair.Value, Pair.Key);
	}

	if (false == ::IsValid(TabButtonContainer))
	{
		return;
	}

	for (UWidget* Child : TabButtonContainer->GetAllChildren())
	{
		if (UDC1TabHeaderButton* TabButton = Cast<UDC1TabHeaderButton>(Child))
		{
			FGameplayTag TabTag = TabButton->GetTabTag();

			TabButtons.Add(TabTag, TabButton);
			TabButton->OnClickedDelegate.AddWeakLambda(this, [this, Tag = TabTag]()
				{
					SelectTab(Tag);
				});
		}
	}

	ButtonInitialize();
}

void UDC1TabWidget::ButtonInitialize()
{
	for (const TPair<FGameplayTag, TObjectPtr<UDC1TabHeaderButton>>& Pair : TabButtons)
	{
		UDC1TabHeaderButton* TabButton = Pair.Value.Get();
		if (true == ::IsValid(TabButton))
		{
			TabButton->SetState(EDC1TabButtonState::Inactive);
		}
	}
}

void UDC1TabWidget::CloseWidget(const FKey& InputKey)
{
	if (false == CurrentTab.IsValid())
	{
		return;
	}

	const FDC1KeyList* KeyList = TabCloseKeys.Find(CurrentTab);
	if (nullptr == KeyList)
	{
		return;
	}

	if (true == KeyList->Keys.Contains(InputKey))
	{
		UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
		if (false == ::IsValid(UIManager))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
			return;
		}

		UIManager->PopScreen();
	}
}

UDC1TabHeaderButton* UDC1TabWidget::GetTabButton(const FGameplayTag& Tab) const
{
	const TObjectPtr<UDC1TabHeaderButton>* TabButtonPtr = TabButtons.Find(Tab);
	if (nullptr == TabButtonPtr)
	{
		return nullptr;
	}

	UDC1TabHeaderButton* TabButton = TabButtonPtr->Get();
	if (false == ::IsValid(TabButton))
	{
		return nullptr;
	}

	return TabButton;
}


void UDC1TabWidget::HandleInputKey(const FKey& InputKey)
{
	if (TabToSwitchIndex.Num() <= 0)
	{
		return;
	}

	if (true == PrevTabKeys.Keys.Contains(InputKey))
	{
		ChangeTab(false);
		return;
	}

	if (true == NextTabKeys.Keys.Contains(InputKey))
	{
		ChangeTab(true);
		return;
	}

	if (true == CurrentTab.IsValid())
	{
		CloseWidget(InputKey);
		return;
	}
}

bool UDC1TabWidget::SelectTab(const FGameplayTag& Tab)
{
	if (false == CanSelectTab(Tab))
	{
		return false;
	}

	if (CurrentTab == Tab)
	{
		return true;
	}

	int32* Index = TabToSwitchIndex.Find(Tab);
	if (nullptr == Index)
	{
		return false;
	}

	if (false == ::IsValid(TabSwitcher))
	{
		return false;
	}

	UDC1TabHeaderButton* ChangeTabButton = GetTabButton(Tab);
	if (false == ::IsValid(ChangeTabButton))
	{
		return false;
	}

	UDC1TabHeaderButton* CurrentTabButton = GetTabButton(CurrentTab);	
	if (true == ::IsValid(CurrentTabButton))
	{
		CurrentTabButton->SetState(EDC1TabButtonState::Inactive);
	}

	ChangeTabButton->SetState(EDC1TabButtonState::Active);
	CurrentTab = Tab;
	TabSwitcher->SetActiveWidgetIndex(*Index);
	
	return true;
}

void UDC1TabWidget::EnableTab(const FGameplayTag& Tab)
{
	UDC1TabHeaderButton* TabButton = GetTabButton(Tab);

	if (true == ::IsValid(TabButton) && false == TabButton->CanSelect())
	{
		TabButton->SetState(EDC1TabButtonState::Inactive);
	}
}

bool UDC1TabWidget::CanSelectTab(const FGameplayTag& Tab) const
{
	UDC1TabHeaderButton* TabButton = GetTabButton(Tab);

	if (false == ::IsValid(TabButton))
	{
		return false;
	}

	return TabButton->CanSelect();
}

void UDC1TabWidget::ChangeTab(bool IsNext)
{
	const int32* CurrentIndex = TabToSwitchIndex.Find(CurrentTab);
	if (nullptr == CurrentIndex)
	{
		UE_LOG(LogTemp, Warning, TEXT("DC1TabWidget :: None bind Tab to Index"));
		return;
	}

	int32 NumsIndex = TabToSwitchIndex.Num();

	int32 Step = IsNext ? 1 : -1;

	for (int32 Offset = 1; Offset < NumsIndex; ++Offset)
	{
		// 음수 방지 () + NumIndex % NumsIndex
		int32 ChangeIndex = (((*(CurrentIndex)+(Step * Offset)) % NumsIndex) + NumsIndex) % NumsIndex;

		const FGameplayTag* ChangeTab = SwitchIndexToTab.Find(ChangeIndex);
		if (nullptr == ChangeTab)
		{
			UE_LOG(LogTemp, Warning, TEXT("DC1TabWidget :: None bind Tab to Index"));
			continue;
		}

		if (true == SelectTab(*ChangeTab))
		{
			break;
		}
	}
}

void UDC1TabWidget::SelectFirstInactiveTab()
{
	int32 NumsIndex = TabToSwitchIndex.Num();

	for (int32 Index = 0; Index < NumsIndex; ++Index)
	{
		const FGameplayTag* ChangeTab = SwitchIndexToTab.Find(Index);
		if (nullptr == ChangeTab)
		{
			UE_LOG(LogTemp, Warning, TEXT("DC1TabWidget :: None bind Tab to Index"));
			continue;
		}

		if (true == SelectTab(*ChangeTab))
		{
			break;
		}
	}
}
