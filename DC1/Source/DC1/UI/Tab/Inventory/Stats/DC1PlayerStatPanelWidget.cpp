#include "UI/Tab/Inventory/Stats/DC1PlayerStatPanelWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/LocalPlayer.h"

#include "UI/Tab/Inventory/Stats/DC1PlayerStatWidget.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Player/DC1PlayerStatsViewModel.h"

void UDC1PlayerStatPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (UDC1ViewModelSubsystem* ViewModelSubsystem = GetOwningLocalPlayer() ? GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>() : nullptr)
	{
		CachedViewModel = ViewModelSubsystem->GetViewModel<UDC1PlayerStatsViewModel>();
	}

	StatWidgets = { PlayerStat_Health, PlayerStat_Stamina, PlayerStat_Attack, PlayerStat_Defense, PlayerStat_WorkSpeed, PlayerStat_Weight };
	for (UDC1PlayerStatWidget* StatWidget : StatWidgets)
	{
		if (::IsValid(StatWidget))
		{
			StatWidget->SetViewModel(CachedViewModel);
		}
	}

	if (::IsValid(Button_EnhanceStats))
	{
		Button_EnhanceStats->OnClicked.AddDynamic(this, &ThisClass::HandleEnhanceStatsClicked);
	}

	if (::IsValid(Button_Confirm))
	{
		Button_Confirm->OnClicked.AddDynamic(this, &ThisClass::HandleConfirmClicked);
	}

	if (::IsValid(Button_Cancel))
	{
		Button_Cancel->OnClicked.AddDynamic(this, &ThisClass::HandleCancelClicked);
	}
}

void UDC1PlayerStatPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (::IsValid(CachedViewModel))
	{
		CachedViewModel->OnRemainingStatPointsChanged.RemoveAll(this);
		CachedViewModel->OnRemainingStatPointsChanged.AddUObject(this, &ThisClass::HandleRemainingStatPointsChanged);
		HandleRemainingStatPointsChanged(CachedViewModel->GetRemainingStatPoints());
	}

	SetEnhanceMode(false);
}

void UDC1PlayerStatPanelWidget::NativeDestruct()
{
	// 확정하지 않고 화면을 닫으면 배분은 버린다
	if (::IsValid(CachedViewModel))
	{
		CachedViewModel->OnRemainingStatPointsChanged.RemoveAll(this);
		CachedViewModel->CancelPending();
	}

	if (bEnhanceMode)
	{
		bEnhanceMode = false;
		OnEnhanceModeChanged.Broadcast(false);
	}

	Super::NativeDestruct();
}

void UDC1PlayerStatPanelWidget::SetEnhanceMode(bool bInEnhanceMode)
{
	const bool bChanged = bEnhanceMode != bInEnhanceMode;
	bEnhanceMode = bInEnhanceMode;

	for (UDC1PlayerStatWidget* StatWidget : StatWidgets)
	{
		if (::IsValid(StatWidget))
		{
			StatWidget->SetEnhanceMode(bEnhanceMode);
		}
	}

	RefreshButtons();

	if (bChanged)
	{
		OnEnhanceModeChanged.Broadcast(bEnhanceMode);
	}
}

void UDC1PlayerStatPanelWidget::RefreshButtons()
{
	const bool bHasViewModel = ::IsValid(CachedViewModel);

	if (::IsValid(Button_EnhanceStats))
	{
		Button_EnhanceStats->SetVisibility(bEnhanceMode ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		Button_EnhanceStats->SetIsEnabled(bHasViewModel && 0 < CachedViewModel->GetRemainingStatPoints());
	}

	const ESlateVisibility ConfirmVisibility = bEnhanceMode ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	if (::IsValid(Button_Confirm))
	{
		Button_Confirm->SetVisibility(ConfirmVisibility);
		Button_Confirm->SetIsEnabled(bHasViewModel && CachedViewModel->HasPending());
	}

	if (::IsValid(Button_Cancel))
	{
		Button_Cancel->SetVisibility(ConfirmVisibility);
	}
}

void UDC1PlayerStatPanelWidget::HandleRemainingStatPointsChanged(int32 InRemainingPoints)
{
	if (::IsValid(TextBlock_StatPoints))
	{
		TextBlock_StatPoints->SetText(FText::AsNumber(InRemainingPoints));
	}

	RefreshButtons();
}

void UDC1PlayerStatPanelWidget::HandleEnhanceStatsClicked()
{
	SetEnhanceMode(true);
}

void UDC1PlayerStatPanelWidget::HandleConfirmClicked()
{
	if (::IsValid(CachedViewModel))
	{
		CachedViewModel->ConfirmPending();
	}

	SetEnhanceMode(false);
}

void UDC1PlayerStatPanelWidget::HandleCancelClicked()
{
	if (::IsValid(CachedViewModel))
	{
		CachedViewModel->CancelPending();
	}

	SetEnhanceMode(false);
}
