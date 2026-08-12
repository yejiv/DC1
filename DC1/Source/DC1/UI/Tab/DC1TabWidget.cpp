// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/DC1TabWidget.h"

#include "UI/Button/AnimButton/Tab/DC1TabHeaderButton.h"

#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Components/WidgetSwitcher.h"

void UDC1TabWidget::NativeConstruct()
{
	Super::NativeConstruct();

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
}

void UDC1TabWidget::HandleInputKey(const FKey& InputKey)
{
	if (0 >= TabToSwitchIndex.Num())
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

void UDC1TabWidget::SelectTab(const FGameplayTag& Tab)
{
	int32* Index = TabToSwitchIndex.Find(Tab);
	if (nullptr == Index)
	{
		return;
	}

	if (false == ::IsValid(TabSwitcher))
	{
		return;
	}

	// ChangeButton
	TObjectPtr<UDC1TabHeaderButton>* ChangeTabPtr = TabButtons.Find(Tab);
	if (nullptr == ChangeTabPtr)
	{
		return;
	}

	UDC1TabHeaderButton* ChangeTabButton = ChangeTabPtr->Get();

	// CurrentButton
	UDC1TabHeaderButton* CurrentTabButton = nullptr;
	TObjectPtr<UDC1TabHeaderButton>* CurrentTabPtr = TabButtons.Find(CurrentTab);
	if (nullptr != CurrentTabPtr)
	{
		CurrentTabButton = CurrentTabPtr->Get();
	}

	if (nullptr != CurrentTabButton)
	{
		CurrentTabButton->SetActive(false);
	}

	ChangeTabButton->SetActive(true);

	CurrentTab = Tab;
	TabSwitcher->SetActiveWidgetIndex(*Index);
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
	int32 ChangeIndex = IsNext ? (*(CurrentIndex) + 1) % NumsIndex :
								((*(CurrentIndex) - 1) + NumsIndex ) % NumsIndex;

	const FGameplayTag* ChangeTab = SwitchIndexToTab.Find(ChangeIndex);
	if (nullptr == ChangeTab)
	{
		UE_LOG(LogTemp, Warning, TEXT("DC1TabWidget :: None bind Tab to Index"));
		return;
	}

	SelectTab(*ChangeTab);
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
