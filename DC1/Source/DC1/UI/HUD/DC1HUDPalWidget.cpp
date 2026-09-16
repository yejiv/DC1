// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/DC1HUDPalWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/LocalPlayer.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Pal/DC1PalViewModel.h"
#include "UI/HUD/DC1HUDPalIcon.h"

void UDC1HUDPalWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RegisterPalIcons();
	BindViewModel();
}

void UDC1HUDPalWidget::NativeDestruct()
{
	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1HUDPalWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);

	if (Animation == Shift_Left || Animation == Shift_Right)
	{
		ResetIcons();
	}
}

void UDC1HUDPalWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		PalViewModel = VMSubsystem->GetViewModel<UDC1PalViewModel>();
		if (false == PalViewModel.IsValid())
			return;

		PalViewModel->OnSlotShifted.AddUObject(this, &ThisClass::HandleSlotShifted);
		PalViewModel->OnInfoChanged.AddUObject(this, &ThisClass::HandleInfoChanged);

		HandleInfoChanged();
	}
}

void UDC1HUDPalWidget::UnbindViewModel()
{
	if (false == PalViewModel.IsValid())
		return;

	PalViewModel->OnSlotShifted.RemoveAll(this);
	PalViewModel->OnInfoChanged.RemoveAll(this);
	PalViewModel.Reset();
}

void UDC1HUDPalWidget::RegisterPalIcons()
{
	PalIcons.Reset();
	RestRenderTransform.Reset();
	RestSlotSize.Reset();
	CanvasPanelSlots.Reset();

	for (UWidget* Child : Canvas_PalIcons->GetAllChildren())
	{
		if (UDC1HUDPalIcon* Icon = Cast<UDC1HUDPalIcon>(Child))
		{
			if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Icon->Slot))
			{
				PalIcons.Add(Icon);
				CanvasPanelSlots.Add(CanvasSlot);
				RestRenderTransform.Add(Icon->GetRenderTransform());
				RestSlotSize.Add(CanvasSlot->GetSize());
			}
		}
	}
}

void UDC1HUDPalWidget::ResetIcons()
{
	int32 PalIconsNum = PalIcons.Num();
	for (int32 i = 0; i < PalIconsNum; ++i)
	{
		PalIcons[i]->SetRenderTransform(RestRenderTransform[i]);
		CanvasPanelSlots[i]->SetSize(RestSlotSize[i]);

		if (PalViewModel.IsValid())
		{
			PalIcons[i]->SetIcon(PalViewModel->GetPalIconTexture(i - PalIconsNum / 2));
		}
	}
}

void UDC1HUDPalWidget::HandleSlotShifted(EDC1PalSlotShift InShift)
{
	if (InShift == EDC1PalSlotShift::None)
		return;
	
	if (IsValid(CurrentAnim) && IsAnimationPlaying(CurrentAnim))
	{
		StopAnimation(CurrentAnim);
		ResetIcons();
	}

	if (InShift == EDC1PalSlotShift::Prev)
	{
		CurrentAnim = Shift_Right;
	}
	else if(InShift == EDC1PalSlotShift::Next)
	{
		CurrentAnim = Shift_Left;
	}
	PlayAnimation(CurrentAnim);
}

void UDC1HUDPalWidget::HandleInfoChanged()
{

}
