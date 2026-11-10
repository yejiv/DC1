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
	PalPartyWidget->OnSelectedPal.BindUObject(this, &UDC1PalInfoWidget::HandleSelectedPal);
	PalDetailWidget->OnBackClicked.BindUObject(this, &UDC1PalInfoWidget::CloseDetail);
}

void UDC1PalInfoWidget::NativeDestruct()
{
	PalPartyWidget->OnChangedFocusingPal.Unbind();
	PalPartyWidget->OnSelectedPal.Unbind();
	PalDetailWidget->OnBackClicked.Unbind();

	Super::NativeDestruct();
}

void UDC1PalInfoWidget::OpenDetail(int32 InSlotIndex)
{
	PalDetailWidget->SetPalInfo(InSlotIndex);
	PalDetailWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UDC1PalInfoWidget::CloseDetail()
{
	PalDetailWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UDC1PalInfoWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	PalPartyWidget->RefreshPalPartySlots();

	CloseDetail();

	Image_PalPreview->SetVisibility(ESlateVisibility::Collapsed);
}

void UDC1PalInfoWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if(true == PreviewSubsystem.IsValid())
		PreviewSubsystem->EndPreview(EDC1PreviewType::Pal);
}

bool UDC1PalInfoWidget::IsDetailOpen() const
{
	return ESlateVisibility::Collapsed != PalDetailWidget->GetVisibility();
}

void UDC1PalInfoWidget::HandleChangedFocusingPal(const FGameplayTag& SpeciesTag)
{
	// 상세 창이 열려 있는 동안은 호버로 미리보기를 바꾸지 않음 (선택한 팰 유지)
	if (true == IsDetailOpen())
		return;

	FocusPal(SpeciesTag);
}

void UDC1PalInfoWidget::HandleSelectedPal(int32 SlotIndex, const FGameplayTag& SpeciesTag)
{
	// 빈 칸 클릭은 무시
	if (false == SpeciesTag.IsValid())
		return;

	// 상세 창이 열려 있어도 다른 팰을 선택하면 그 팰로 갱신
	FocusPal(SpeciesTag);
	OpenDetail(SlotIndex);
}

void UDC1PalInfoWidget::FocusPal(const FGameplayTag& SpeciesTag)
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
