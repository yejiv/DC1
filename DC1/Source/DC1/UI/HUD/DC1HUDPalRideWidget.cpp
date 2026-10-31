// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/DC1HUDPalRideWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Components/DynamicEntryBox.h"
#include "Components/TextBlock.h"
#include "Engine/LocalPlayer.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "UI/HUD/DC1HUDPalRideSkillRow.h"
#include "ViewModel/Pal/DC1PalRideViewModel.h"

void UDC1HUDPalRideWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
}

void UDC1HUDPalRideWidget::NativeDestruct()
{
	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1HUDPalRideWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		RideViewModel = VMSubsystem->GetViewModel<UDC1PalRideViewModel>();
		if (false == RideViewModel.IsValid())
			return;

		RideViewModel->OnRideInfoChanged.AddUObject(this, &ThisClass::HandleRideInfoChanged);
		RideViewModel->OnRideHoldChanged.AddUObject(this, &ThisClass::HandleRideHoldChanged);
		RideViewModel->OnRideHoldCompleted.AddUObject(this, &ThisClass::HandleRideHoldCompleted);

		HandleRideInfoChanged();
	}
}

void UDC1HUDPalRideWidget::UnbindViewModel()
{
	if (false == RideViewModel.IsValid())
		return;

	RideViewModel->OnRideInfoChanged.RemoveAll(this);
	RideViewModel->OnRideHoldChanged.RemoveAll(this);
	RideViewModel->OnRideHoldCompleted.RemoveAll(this);
	RideViewModel.Reset();
}

void UDC1HUDPalRideWidget::HandleRideInfoChanged()
{
	const bool bAvailable = RideViewModel.IsValid() && RideViewModel->IsRideAvailable();
	SetVisibility(bAvailable ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	if (false == bAvailable)
		return;

	const bool bRiding = RideViewModel->IsRiding();
	Text_Action->SetText(bRiding ? DismountText : RideText);

	SkillContainer->Reset();

	if (true == bRiding)
	{
		const TArray<FText>& SkillNames = RideViewModel->GetRideSkillNames();
		const int32 NumSkills = FMath::Min(SkillNames.Num(), SkillKeyIcons.Num());

		for (int32 Index = 0; Index < NumSkills; ++Index)
		{
			if (UDC1HUDPalRideSkillRow* Row = SkillContainer->CreateEntry<UDC1HUDPalRideSkillRow>())
			{
				Row->SetSkill(SkillNames[Index], SkillKeyIcons[Index]);
			}
		}
	}

	Panel_Skill->SetVisibility((SkillContainer->GetNumEntries() > 0) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UDC1HUDPalRideWidget::HandleRideHoldChanged(bool InPressed)
{
	FDC1AnimPlayInfo PlayInfo;
	PlayInfo.PlayMode = InPressed ? EUMGSequencePlayMode::Forward : EUMGSequencePlayMode::Reverse;

	PlayAnim(PressAnim, PlayInfo);
}

void UDC1HUDPalRideWidget::HandleRideHoldCompleted()
{
	StopAnim(SuccessAnim);
	PlayAnim(SuccessAnim, FDC1AnimPlayInfo());
}
