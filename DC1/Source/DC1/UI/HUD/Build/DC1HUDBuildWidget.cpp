// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/Build/DC1HUDBuildWidget.h"

#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/LocalPlayer.h"

#include "Actor/BuildActor/DC1BuildActor.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "UI/HUD/Build/DC1HUDBuildGuide.h"
#include "UI/HUD/Build/DC1HUDBuildMaterialList.h"
#include "UI/HUD/Build/DC1HUDBuildModeBanner.h"
#include "ViewModel/Build/DC1BuildViewModel.h"

void UDC1HUDBuildWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
}

void UDC1HUDBuildWidget::NativeDestruct()
{
	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1HUDBuildWidget::BindViewModel()
{
	const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	UDC1ViewModelSubsystem* VMSubsystem = (nullptr != LocalPlayer) ? LocalPlayer->GetSubsystem<UDC1ViewModelSubsystem>() : nullptr;
	if (nullptr != VMSubsystem)
	{
		BuildViewModel = VMSubsystem->GetViewModel<UDC1BuildViewModel>();
	}

	if (false == BuildViewModel.IsValid())
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	BuildViewModel->OnBuildModeChanged.AddUObject(this, &ThisClass::HandleBuildModeChanged);
	BuildViewModel->OnBlockReasonChanged.AddUObject(this, &ThisClass::HandleBlockReasonChanged);
	BuildViewModel->OnDismantleTargetChanged.AddUObject(this, &ThisClass::HandleDismantleTargetChanged);

	HandleBuildModeChanged(BuildViewModel->GetBuildMode(), BuildViewModel->GetCurrentBuildTag());
}

void UDC1HUDBuildWidget::UnbindViewModel()
{
	if (false == BuildViewModel.IsValid())
	{
		return;
	}

	BuildViewModel->OnBuildModeChanged.RemoveAll(this);
	BuildViewModel->OnBlockReasonChanged.RemoveAll(this);
	BuildViewModel->OnDismantleTargetChanged.RemoveAll(this);
	BuildViewModel.Reset();
}

void UDC1HUDBuildWidget::HandleBuildModeChanged(EDC1BuildMode InMode, const FGameplayTag& InBuildTag)
{
	if (true == ::IsValid(MaterialList))
	{
		MaterialList->Clear();
	}

	if (EDC1BuildMode::Default == InMode)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (true == ::IsValid(ModeBanner))
	{
		ModeBanner->SetMode(InMode);
	}

	if (true == ::IsValid(Image_Center))
	{
		UTexture2D* CenterTexture = ModeCenterTextures.FindRef(InMode);
		if (nullptr != CenterTexture)
		{
			Image_Center->SetBrushFromTexture(CenterTexture);
		}
	}

	if (true == ::IsValid(GuideBox))
	{
		GuideBox->SetMode(InMode);
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);

	const EDC1BuildBlockReason Reason = BuildViewModel.IsValid() ? BuildViewModel->GetBlockReason() : EDC1BuildBlockReason::None;
	HandleBlockReasonChanged(Reason);

	if (EDC1BuildMode::Placement == InMode)
	{
		SetPlacementInfo(InBuildTag);
		return;
	}

	SetDismantleInfo(BuildViewModel.IsValid() ? BuildViewModel->GetDismantleTarget() : nullptr);
}

void UDC1HUDBuildWidget::HandleDismantleTargetChanged(ADC1BuildActor* InTarget)
{
	if (false == BuildViewModel.IsValid() || EDC1BuildMode::Dismantle != BuildViewModel->GetBuildMode())
	{
		return;
	}

	SetDismantleInfo(InTarget);
}

void UDC1HUDBuildWidget::HandleBlockReasonChanged(EDC1BuildBlockReason InReason)
{
	if (false == ::IsValid(BlockReasonBox))
	{
		return;
	}

	if (EDC1BuildBlockReason::None == InReason)
	{
		BlockReasonBox->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (true == ::IsValid(Text_BlockReason))
	{
		Text_BlockReason->SetText(BlockReasonTexts.FindRef(InReason));
	}

	BlockReasonBox->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UDC1HUDBuildWidget::SetPlacementInfo(const FGameplayTag& InBuildTag)
{
	const FDC1BuildRow* BuildRow = BuildViewModel.IsValid() ? BuildViewModel->FindBuildRow(InBuildTag) : nullptr;

	if (true == ::IsValid(Text_BuildName))
	{
		Text_BuildName->SetText((nullptr != BuildRow) ? BuildRow->DisplayName : FText::GetEmpty());
	}

	if (nullptr == BuildRow || false == ::IsValid(MaterialList))
	{
		return;
	}

	MaterialList->SetCosts(BuildRow->BuildCosts);
}

void UDC1HUDBuildWidget::SetDismantleInfo(const ADC1BuildActor* InTarget)
{
	if (false == ::IsValid(InTarget))
	{
		if (true == ::IsValid(Text_BuildName))
		{
			Text_BuildName->SetText(FText::GetEmpty());
		}

		if (true == ::IsValid(MaterialList))
		{
			MaterialList->Clear();
		}

		return;
	}

	const FDC1BuildActorData& BuildData = InTarget->GetBuildActorData();
	const FDC1BuildRow* BuildRow = BuildViewModel.IsValid() ? BuildViewModel->FindBuildRow(BuildData.BuildTag) : nullptr;

	if (true == ::IsValid(Text_BuildName))
	{
		Text_BuildName->SetText((nullptr != BuildRow) ? BuildRow->DisplayName : FText::GetEmpty());
	}

	if (true == ::IsValid(MaterialList))
	{
		MaterialList->SetAmounts(BuildData.BuildCosts);
	}
}
