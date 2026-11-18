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
		RideViewModel->OnRideSkillCooldownChanged.AddUObject(this, &ThisClass::HandleRideSkillCooldownChanged);

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
	RideViewModel->OnRideSkillCooldownChanged.RemoveAll(this);
	RideViewModel.Reset();
}

void UDC1HUDPalRideWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (false == RideViewModel.IsValid())
		return;

	for (int32 Index = 0; Index < SkillRows.Num(); ++Index)
	{
		if (nullptr != SkillRows[Index] && true == RideViewModel->IsRideSkillCoolingDown(Index))
		{
			SkillRows[Index]->SetCooldownRatio(RideViewModel->GetRideSkillCooldownRatio(Index));
		}
	}
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
	SkillRows.Reset();

	if (true == bRiding)
	{
		const TArray<FText>& SkillNames = RideViewModel->GetRideSkillNames();
		const int32 NumSkills = FMath::Min(SkillNames.Num(), SkillKeyIcons.Num());

		for (int32 Index = 0; Index < NumSkills; ++Index)
		{
			// 자리는 스킬 순서 번호와 맞춘다
			UDC1HUDPalRideSkillRow* Row = SkillContainer->CreateEntry<UDC1HUDPalRideSkillRow>();
			SkillRows.Add(Row);

			if (nullptr != Row)
			{
				Row->SetSkill(SkillNames[Index], SkillKeyIcons[Index]);
				Row->SetCoolingDown(RideViewModel->IsRideSkillCoolingDown(Index));
				Row->SetCooldownRatio(RideViewModel->GetRideSkillCooldownRatio(Index));
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

void UDC1HUDPalRideWidget::HandleRideSkillCooldownChanged(int32 InIndex, bool InStarted)
{
	if (false == SkillRows.IsValidIndex(InIndex) || nullptr == SkillRows[InIndex])
		return;

	UDC1HUDPalRideSkillRow* Row = SkillRows[InIndex];
	Row->SetCoolingDown(InStarted);

	if (true == InStarted)
	{
		Row->PlayCooldownFlash();
	}
}
