// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/PalInfo/DC1PalInfoWidget.h"

#include "Subsystem/PlayerPreview/DC1PreviewSubsystem.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "Data/DA/UI/DC1UIResourceData.h"
#include "System/DC1AssetManager.h"
#include "Tags/DC1GamePlayTags.h"

#include "UI/Tab/PalInfo/DC1PalPartyWidget.h"
#include "UI/Tab/PalInfo/DC1PalDetailWidget.h"
#include "ViewModel/Pal/DC1PalViewModel.h"

#include "Components/Image.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"

void UDC1PalInfoWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PreviewSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1PreviewSubsystem>();
	if (nullptr == PreviewSubsystem)
		return;

	const UDC1UIResourceData* ResourceData = UDC1AssetManager::GetAssetByTag<UDC1UIResourceData>(DC1AssetTags::Asset_UI_Resource_Common);
	if (nullptr == ResourceData || nullptr == ResourceData->PalPreviewMaterial)
		return;

	PalPreviewMID = UMaterialInstanceDynamic::Create(ResourceData->PalPreviewMaterial, this);
	if (nullptr != PalPreviewMID)
	{
		PalPreviewMID->SetTextureParameterValue(TextureParamName, PreviewSubsystem->GetRenderTarget(EDC1PreviewType::Pal));
	}
}

void UDC1PalInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PalPartyWidget->OnChangedFocusingPal.BindUObject(this, &UDC1PalInfoWidget::HandleChangedFocusingPal);
}

void UDC1PalInfoWidget::NativeDestruct()
{
	PalPartyWidget->OnChangedFocusingPal.Unbind();

	Super::NativeDestruct();
}

void UDC1PalInfoWidget::ShowPartyPanel()
{
	PalPartyWidget->RefreshPalPartySlots();

	WidgetSwitcher_Panel->SetActiveWidget(PalPartyWidget.Get());

	Image_PalPreview->SetVisibility(ESlateVisibility::Collapsed);
}

void UDC1PalInfoWidget::ShowDetailPanel()
{
	WidgetSwitcher_Panel->SetActiveWidget(PalDetailWidget.Get());
}

void UDC1PalInfoWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	ShowPartyPanel();
}

void UDC1PalInfoWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if(true == PreviewSubsystem.IsValid())
		PreviewSubsystem->EndPreview(EDC1PreviewType::Pal);
}

void UDC1PalInfoWidget::HandleChangedFocusingPal(const FGameplayTag& SpeciesTag)
{
	if (false == SpeciesTag.IsValid())
	{
		Image_PalPreview->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	if (false == PreviewSubsystem.IsValid())
		return;
	if (nullptr == PalPreviewMID)
		return;

	Image_PalPreview->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	PreviewSubsystem->BeginPalPreview(SpeciesTag);
	Image_PalPreview->SetBrushFromMaterial(PalPreviewMID);
}
