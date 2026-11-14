#include "UI/HUD/DC1HUDWarningWidget.h"

#include "UI/HUD/DC1HUDWarningRow.h"
#include "Components/DynamicEntryBox.h"
#include "Engine/LocalPlayer.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Player/DC1PlayerStatusViewModel.h"

void UDC1HUDWarningWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
}

void UDC1HUDWarningWidget::NativeDestruct()
{
	UnbindViewModel();

	WarningBox->Reset();
	ActiveRows.Empty();

	Super::NativeDestruct();
}

void UDC1HUDWarningWidget::SetWarning(EDC1HUDWarningType InType, const FText& InText, const FLinearColor& InColor)
{
	TObjectPtr<UDC1HUDWarningRow>* FoundRow = ActiveRows.Find(InType);
	UDC1HUDWarningRow* Row = (nullptr != FoundRow) ? FoundRow->Get() : nullptr;

	if (false == ::IsValid(Row))
	{
		Row = WarningBox->CreateEntry<UDC1HUDWarningRow>();
		if (false == ::IsValid(Row))
		{
			return;
		}

		ActiveRows.Add(InType, Row);
	}

	Row->SetWarning(InText, InColor);
}

void UDC1HUDWarningWidget::ClearWarning(EDC1HUDWarningType InType)
{
	TObjectPtr<UDC1HUDWarningRow> Row = nullptr;
	if (false == ActiveRows.RemoveAndCopyValue(InType, Row))
	{
		return;
	}

	WarningBox->RemoveEntry(Row);
}

void UDC1HUDWarningWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		StatusViewModel = VMSubsystem->GetViewModel<UDC1PlayerStatusViewModel>();
		if (false == StatusViewModel.IsValid())
		{
			return;
		}

		StatusViewModel->OnSatietyChanged.AddUObject(this, &ThisClass::HandleSatietyChanged);

		HandleSatietyChanged(StatusViewModel->GetSatiety(), StatusViewModel->GetMaxSatiety());
	}
}

void UDC1HUDWarningWidget::UnbindViewModel()
{
	if (false == StatusViewModel.IsValid())
	{
		return;
	}

	StatusViewModel->OnSatietyChanged.RemoveAll(this);
	StatusViewModel.Reset();
}

void UDC1HUDWarningWidget::HandleSatietyChanged(float Current, float Max)
{
	if (StatusViewModel->GetSatietyPercent() <= StatusViewModel->GetHungryRatio())
	{
		SetWarning(EDC1HUDWarningType::Hunger, HungryText, WarningColor);
	}
	else
	{
		ClearWarning(EDC1HUDWarningType::Hunger);
	}
	
	if (Current <= 0.f)
	{
		SetWarning(EDC1HUDWarningType::Starve, StarvingText, DangeColor);
	}
	else
	{
		ClearWarning(EDC1HUDWarningType::Starve);
	}
}
