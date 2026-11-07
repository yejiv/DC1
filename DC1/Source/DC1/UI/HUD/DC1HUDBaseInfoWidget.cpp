// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/DC1HUDBaseInfoWidget.h"

#include "Components/DynamicEntryBox.h"
#include "Components/TextBlock.h"
#include "Engine/LocalPlayer.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "UI/HUD/DC1HUDBaseInfoRow.h"
#include "ViewModel/Pal/DC1BaseInfoViewModel.h"

void UDC1HUDBaseInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
}

void UDC1HUDBaseInfoWidget::NativeDestruct()
{
	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1HUDBaseInfoWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		BaseInfoViewModel = VMSubsystem->GetViewModel<UDC1BaseInfoViewModel>();
		if (false == BaseInfoViewModel.IsValid())
			return;

		BaseInfoViewModel->OnBaseInfoChanged.AddUObject(this, &ThisClass::HandleBaseInfoChanged);

		HandleBaseInfoChanged();
	}
}

void UDC1HUDBaseInfoWidget::UnbindViewModel()
{
	if (false == BaseInfoViewModel.IsValid())
		return;

	BaseInfoViewModel->OnBaseInfoChanged.RemoveAll(this);
	BaseInfoViewModel.Reset();
}

void UDC1HUDBaseInfoWidget::HandleBaseInfoChanged()
{
	const bool bInBase = BaseInfoViewModel.IsValid() && BaseInfoViewModel->IsInBase();
	SetVisibility(bInBase ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	if (false == bInBase)
		return;

	Text_PalCount->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), BaseInfoViewModel->GetPalCount(), BaseInfoViewModel->GetCapacity())));

	RowContainer->Reset();

	for (const FDC1BaseInfoRow& RowInfo : BaseInfoViewModel->GetRows())
	{
		UDC1HUDBaseInfoRow* Row = RowContainer->CreateEntry<UDC1HUDBaseInfoRow>();
		if (nullptr == Row)
			continue;

		const FText* ConditionText = &HealthyText;
		switch (RowInfo.Condition)
		{
		case EDC1BasePalCondition::Resting:	ConditionText = &RestingText;	break;
		case EDC1BasePalCondition::Tired:	ConditionText = &TiredText;		break;
		case EDC1BasePalCondition::Capable:	ConditionText = &CapableText;	break;
		default:																break;
		}

		Row->SetRow(RowInfo.Icon, *ConditionText, RowInfo.bWorking);
	}
}
