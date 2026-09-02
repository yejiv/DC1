// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/DC1TabWidget.h"

#include "UI/Button/AnimButton/Tab/DC1TabHeaderButton.h"
#include "UI/Tab/Inventory/DC1InventoryWidget.h"
#include "UI/Tab/TechTree/DC1TechTreeWidget.h"

#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Tags/UI/DC1GameplayTag_UI.h"

UDC1TabHeaderButton* UDC1TabWidget::GetTabButton(const FGameplayTag& InTab) const
{
	const TObjectPtr<UDC1TabHeaderButton>* TabButtonPtr = TabButtons.Find(InTab);
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

void UDC1TabWidget::CloseWidget(const FKey& InKey)
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

	if (true == KeyList->Keys.Contains(InKey))
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

void UDC1TabWidget::HandleInputKey(const FKey& InKey)
{
	if (TabToSwitchIndex.Num() <= 0)
	{
		return;
	}

	if (true == PrevTabKeys.Keys.Contains(InKey))
	{
		ChangeTab(false);
		return;
	}

	if (true == NextTabKeys.Keys.Contains(InKey))
	{
		ChangeTab(true);
		return;
	}

	if (true == CurrentTab.IsValid())
	{
		CloseWidget(InKey);
		return;
	}
}

bool UDC1TabWidget::SelectTab(const FGameplayTag& InTab)
{
	if (false == CanSelectTab(InTab))
	{
		return false;
	}

	if (CurrentTab == InTab)
	{
		return true;
	}

	int32* Index = TabToSwitchIndex.Find(InTab);
	if (nullptr == Index)
	{
		return false;
	}

	if (false == ::IsValid(TabSwitcher))
	{
		return false;
	}

	UDC1TabHeaderButton* ChangeTabButton = GetTabButton(InTab);
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
	CurrentTab = InTab;
	TabSwitcher->SetActiveWidgetIndex(*Index);
	
	return true;
}

void UDC1TabWidget::EnableTab(const FGameplayTag& InTab)
{
	UDC1TabHeaderButton* TabButton = GetTabButton(InTab);

	if (true == ::IsValid(TabButton) && false == TabButton->CanSelect())
	{
		TabButton->SetState(EDC1TabButtonState::Inactive);
	}
}

bool UDC1TabWidget::CanSelectTab(const FGameplayTag& InTab) const
{
	UDC1TabHeaderButton* TabButton = GetTabButton(InTab);

	if (false == ::IsValid(TabButton))
	{
		return false;
	}

	return TabButton->CanSelect();
}

void UDC1TabWidget::ChangeTab(bool InIsNext)
{
	const int32* CurrentIndex = TabToSwitchIndex.Find(CurrentTab);
	if (nullptr == CurrentIndex)
	{
		UE_LOG(LogTemp, Warning, TEXT("DC1TabWidget :: None bind InTab to Index"));
		return;
	}

	int32 NumsIndex = TabToSwitchIndex.Num();

	int32 Step = InIsNext ? 1 : -1;

	for (int32 Offset = 1; Offset < NumsIndex; ++Offset)
	{
		// 음수 방지 () + NumIndex % NumsIndex
		int32 ChangeIndex = (((*(CurrentIndex)+(Step * Offset)) % NumsIndex) + NumsIndex) % NumsIndex;

		const FGameplayTag* ChangeTab = SwitchIndexToTab.Find(ChangeIndex);
		if (nullptr == ChangeTab)
		{
			UE_LOG(LogTemp, Warning, TEXT("DC1TabWidget :: None bind InTab to Index"));
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
	if (true == CanSelectTab(CurrentTab))
	{
		return;
	}

	int32 NumsIndex = TabToSwitchIndex.Num();

	for (int32 Index = 0; Index < NumsIndex; ++Index)
	{
		const FGameplayTag* ChangeTab = SwitchIndexToTab.Find(Index);
		if (nullptr == ChangeTab)
		{
			UE_LOG(LogTemp, Warning, TEXT("DC1TabWidget :: None bind InTab to Index"));
			continue;
		}

		if (true == SelectTab(*ChangeTab))
		{
			break;
		}
	}
}

UDC1InventoryWidget* UDC1TabWidget::GetInventoryWidget() const
{
	return GetTabWidget<UDC1InventoryWidget>(DC1UITags::UI_Main_Tab_Inventory);
}

UDC1TechTreeWidget* UDC1TabWidget::GetTechTreeWidget() const
{
	return GetTabWidget<UDC1TechTreeWidget>(DC1UITags::UI_Main_Tab_TechTree);
}
