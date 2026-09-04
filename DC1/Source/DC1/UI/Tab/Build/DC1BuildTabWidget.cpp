// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/Build/DC1BuildTabWidget.h"

#include "UI/Button/AnimButton/Tab/DC1TabHeaderButton.h"
#include "UI/Tab/Build/DC1BuildWidget.h"

#include "Subsystem/DC1UIManagerSubsystem.h"

#include "Tags/Build/DC1GameplayTag_Build.h"

#include "ViewModel/Build/DC1BuildViewModel.h"
#include "Subsystem/DC1ViewModelSubsystem.h"

void UDC1BuildTabWidget::NativeOnInitialized()
{
	// 베이스가 탭 버튼을 수집하고 ButtonInitialize를 부른다. RefreshAll보다 먼저여야 한다
	Super::NativeOnInitialized();

	ULocalPlayer* OwnerLocalPlayer = GetOwningLocalPlayer();
	if (false == ::IsValid(OwnerLocalPlayer))
	{
		return;
	}

	if (UDC1ViewModelSubsystem* ViewModelSubsystem = OwnerLocalPlayer->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		SetViewModel(ViewModelSubsystem->GetViewModel<UDC1BuildViewModel>());
	}
}

void UDC1BuildTabWidget::CloseWidget(const FKey& InKey)
{
	if (false == BuildCloseKeys.Keys.Contains(InKey))
	{
		return;
	}

	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildTabWidget :: Failed to Find UIManager"));
		return;
	}

	UIManager->PopScreen();
}

void UDC1BuildTabWidget::SetViewModel(UDC1BuildViewModel* InViewModel)
{
	if (CachedViewModel == InViewModel)
	{
		return;
	}

	if (true == ::IsValid(CachedViewModel))
	{
		CachedViewModel->OnBuildStateChanged.RemoveAll(this);
		CachedViewModel->OnBuildUnlocked.RemoveAll(this);
	}

	CachedViewModel = InViewModel;

	if (true == ::IsValid(CachedViewModel))
	{
		CachedViewModel->OnBuildStateChanged.AddUObject(this, &ThisClass::RefreshAll);
		CachedViewModel->OnBuildUnlocked.AddUObject(this, &ThisClass::HandleBuildUnlocked);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildTabWidget :: Can't Find BuildViewModel"));
		return;
	}

	RefreshAll();
}

void UDC1BuildTabWidget::RefreshAll()
{
	if (false == ::IsValid(CachedViewModel) || false == CachedViewModel->HasModel())
	{
		return;
	}

	for (const TPair<FGameplayTag, int32>& Pair : TabToSwitchIndex)
	{
		UDC1BuildWidget* BuildWidget = GetTabWidget<UDC1BuildWidget>(Pair.Key);
		if (false == ::IsValid(BuildWidget))
		{
			continue;
		}

		if (false == IsInit)
		{
			BuildWidget->InitBuild(CachedViewModel);
		}

		if (true == CachedViewModel->IsTabUnlocked(Pair.Key))
		{
			EnableTab(Pair.Key);
		}
		else
		{
			UDC1TabHeaderButton* TabButton = GetTabButton(Pair.Key);
			if (true == ::IsValid(TabButton))
			{
				TabButton->SetState(EDC1TabButtonState::Disable);
			}
		}
	}

	if (false == IsInit)
	{
		IsInit = true;
	}
}

void UDC1BuildTabWidget::HandleBuildUnlocked(const FGameplayTag& InUnlockedTag)
{
	if (false == InUnlockedTag.MatchesTag(DC1BuildTags::Build))
	{
		return;
	}

	const FGameplayTag& RowTag = InUnlockedTag.RequestDirectParent();
	const FGameplayTag& TabTag = RowTag.RequestDirectParent();

	if (false == TabTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildTabWidget :: Invalid TabTag : %s"), *InUnlockedTag.ToString());
		return;
	}

	UDC1BuildWidget* BuildWidget = GetTabWidget<UDC1BuildWidget>(TabTag);
	if (false == ::IsValid(BuildWidget))
	{
		return;
	}

	if (false == BuildWidget->VisibleSlot(RowTag, InUnlockedTag))
	{
		return;
	}

	EnableTab(TabTag);
}