#include "UI/Tab/Inventory/Status/DC1PlayerStatusPanelWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/LocalPlayer.h"

#include "UI/Tab/Inventory/Status/DC1MenuGaugeWidget.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Player/DC1PlayerStatusViewModel.h"

void UDC1PlayerStatusPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (UDC1ViewModelSubsystem* ViewModelSubsystem = GetOwningLocalPlayer() ? GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>() : nullptr)
	{
		CachedViewModel = ViewModelSubsystem->GetViewModel<UDC1PlayerStatusViewModel>();
	}
}

void UDC1PlayerStatusPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
}

void UDC1PlayerStatusPanelWidget::NativeDestruct()
{
	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1PlayerStatusPanelWidget::BindViewModel()
{
	if (false == ::IsValid(CachedViewModel))
	{
		return;
	}

	// 화면을 다시 열 때 중복 구독 방지
	UnbindViewModel();

	CachedViewModel->OnHealthChanged.AddUObject(this, &ThisClass::HandleHealthChanged);
	CachedViewModel->OnShieldChanged.AddUObject(this, &ThisClass::HandleShieldChanged);
	CachedViewModel->OnSatietyChanged.AddUObject(this, &ThisClass::HandleSatietyChanged);
	CachedViewModel->OnLevelChanged.AddUObject(this, &ThisClass::HandleLevelChanged);

	HandleHealthChanged(CachedViewModel->GetHealth(), CachedViewModel->GetMaxHealth());
	HandleShieldChanged(CachedViewModel->GetShield(), CachedViewModel->GetMaxShield());
	HandleSatietyChanged(CachedViewModel->GetSatiety(), CachedViewModel->GetMaxSatiety());
	HandleLevelChanged(CachedViewModel->GetLevel(), CachedViewModel->GetRemainingExp(), CachedViewModel->GetExpPercent());
}

void UDC1PlayerStatusPanelWidget::UnbindViewModel()
{
	if (false == ::IsValid(CachedViewModel))
	{
		return;
	}

	CachedViewModel->OnHealthChanged.RemoveAll(this);
	CachedViewModel->OnShieldChanged.RemoveAll(this);
	CachedViewModel->OnSatietyChanged.RemoveAll(this);
	CachedViewModel->OnLevelChanged.RemoveAll(this);
}

void UDC1PlayerStatusPanelWidget::HandleHealthChanged(float InCurrent, float InMax)
{
	if (::IsValid(MenuGauge_Health))
	{
		MenuGauge_Health->SetValues(InCurrent, InMax);
	}
}

void UDC1PlayerStatusPanelWidget::HandleShieldChanged(float InCurrent, float InMax)
{
	if (::IsValid(MenuGauge_Shield))
	{
		MenuGauge_Shield->SetValues(InCurrent, InMax);
	}
}

void UDC1PlayerStatusPanelWidget::HandleSatietyChanged(float InCurrent, float InMax)
{
	if (::IsValid(MenuGauge_Satiety))
	{
		MenuGauge_Satiety->SetValues(InCurrent, InMax);
	}
}

void UDC1PlayerStatusPanelWidget::HandleLevelChanged(int32 InLevel, int32 InRemainingExp, float InExpPercent)
{
	if (::IsValid(TextBlock_Level))
	{
		TextBlock_Level->SetText(FText::AsNumber(InLevel));
	}

	if (::IsValid(TextBlock_NextExp))
	{
		FNumberFormattingOptions Options;
		Options.UseGrouping = false;
		TextBlock_NextExp->SetText(FText::AsNumber(InRemainingExp, &Options));
	}

	if (::IsValid(ProgressBar_Exp))
	{
		ProgressBar_Exp->SetPercent(InExpPercent);
	}
}
