// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Teleport/DC1TeleportScreenWidget.h"

#include "Subsystem/DC1ViewModelSubsystem.h"
#include "Subsystem/DC1TeleportSubsystem.h"
#include "UI/Teleport/DC1TeleportRowWidget.h"
#include "ViewModel/Teleport/DC1TeleportViewModel.h"

#include "Components/ScrollBox.h"

#include "Define/DC1Macro.h"

void UDC1TeleportScreenWidget::HandleInputKey(const FKey& InKey)
{
	if (InKey != CloseKey)
	{
		return;
	}

	PopScreen();
}

void UDC1TeleportScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
}

void UDC1TeleportScreenWidget::NativeDestruct()
{
	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1TeleportScreenWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (UDC1TeleportSubsystem* Subsystem = GetWorld()->GetSubsystem<UDC1TeleportSubsystem>())
	{
		int32 RowNum = Subsystem->GetTeleportActorNum() + Subsystem->GetMaxBaseNum();
		
		for (int32 i = 0; i < RowNum; ++i)
		{
			CreateRowWidget();
		}
	}
}

void UDC1TeleportScreenWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		TeleportViewModel = VMSubsystem->GetViewModel<UDC1TeleportViewModel>();
		if (nullptr == TeleportViewModel)
			return;

		TeleportViewModel->OnEntriesChanged.AddUObject(this, &ThisClass::HandleEntriesChanged);
		HandleEntriesChanged();
	}
}

void UDC1TeleportScreenWidget::UnbindViewModel()
{
	if (nullptr == TeleportViewModel)
		return;

	TeleportViewModel->OnEntriesChanged.RemoveAll(this);
	TeleportViewModel.Reset();
}

void UDC1TeleportScreenWidget::RefreshRows()
{
	if (nullptr == TeleportViewModel)
		return;

	for (auto& RowWidget : RowWidgets)
	{
		RowWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	TArray<FDC1TeleportEntry> Entries = TeleportViewModel->GetEntries();

	int32 EntriesNum = Entries.Num();
	int32 RowWidgetsNum = RowWidgets.Num();

	if (EntriesNum > RowWidgetsNum)
	{
		LOG_WARNING(TEXT("[Teleport] RowWidget 부족"));
		for (int32 i = 0; i < EntriesNum - RowWidgetsNum; ++i)
			CreateRowWidget();
	}

	for (int32 i = 0; i < EntriesNum; ++i)
	{
		RowWidgets[i]->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		RowWidgets[i]->SetEntry(i, Entries[i]);
	}
}

void UDC1TeleportScreenWidget::HandleEntriesChanged()
{
	RefreshRows();
}

void UDC1TeleportScreenWidget::HandleRowClicked(int32 InRowIndex)
{
	if (nullptr == TeleportViewModel)
		return;

	TeleportViewModel->RequestTeleport(InRowIndex);
}

void UDC1TeleportScreenWidget::CreateRowWidget()
{
	UDC1TeleportRowWidget* RowWidget = CreateWidget<UDC1TeleportRowWidget>(this, RowWidgetClass);
	if (nullptr == RowWidget)
		return;

	RowWidget->OnRowClicked.AddUObject(this, &ThisClass::HandleRowClicked);
	ScrollBox_Points->AddChild(RowWidget);
	RowWidgets.Add(RowWidget);
}
