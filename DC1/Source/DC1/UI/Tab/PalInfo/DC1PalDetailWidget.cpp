// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/PalInfo/DC1PalDetailWidget.h"

#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Pal/DC1PalBoxViewModel.h"

#include "UI/Button/DC1UserButton.h"
#include "UI/Pal/PalBox/DC1PalBoxDetailWidget.h"

void UDC1PalDetailWidget::SetPalInfo(int32 InSlotIndex)
{
	FDC1PalBoxDetailInfo DetailInfo;
	if (true == CachedViewModel.IsValid() && true == CachedViewModel->GetDetailInfo(EDC1PalContainer::Party, InSlotIndex, DetailInfo))
	{
		PalBoxDetail->SetDetailInfo(DetailInfo);
	}
	else
	{
		PalBoxDetail->ShowNoData();
	}
}

void UDC1PalDetailWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();

	Button_Back->OnClickedDelegate.AddUObject(this, &ThisClass::HandleBackClicked);
}

void UDC1PalDetailWidget::NativeDestruct()
{
	Button_Back->OnClickedDelegate.RemoveAll(this);

	UnBindViewModel();

	Super::NativeDestruct();
}

void UDC1PalDetailWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		CachedViewModel = VMSubsystem->GetViewModel<UDC1PalBoxViewModel>();
	}
}

void UDC1PalDetailWidget::UnBindViewModel()
{
	CachedViewModel.Reset();
}

void UDC1PalDetailWidget::HandleBackClicked()
{
	OnBackClicked.ExecuteIfBound();
}
