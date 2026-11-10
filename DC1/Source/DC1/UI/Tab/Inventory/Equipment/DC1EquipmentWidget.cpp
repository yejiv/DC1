// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/Inventory/Equipment/DC1EquipmentWidget.h"

#include "UI/Tab/Inventory/Slot/DC1InventorySlotWidget.h"
#include "ViewModel/Inventory/DC1InventoryViewModel.h"
#include "Component/Inventory/DC1InventoryTypes.h"
#include "Engine/Texture2D.h"
#include "Engine/LocalPlayer.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/Image.h"

#include "Subsystem/PlayerPreview/DC1PreviewSubsystem.h"
#include "Data/DA/UI/DC1UIResourceData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

namespace
{
	// UI 머티리얼(M_UI_CharacterPreview)의 텍스처 파라미터 이름
	const FName PlayerPreviewTextureParam = TEXT("RenderTarget");
}

void UDC1EquipmentWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// DC1EquipSlotLayout 인덱스 순서와 같아야 함
	EquipSlots =
	{
		InventorySlot_Weapon1, InventorySlot_Weapon2, InventorySlot_Weapon3, InventorySlot_Weapon4, InventorySlot_Weapon5, InventorySlot_Weapon6,
		InventorySlot_Head,
		InventorySlot_Body,
		InventorySlot_Shield,
		InventorySlot_Glider,
		InventorySlot_Accessory1, InventorySlot_Accessory2, InventorySlot_Accessory3, InventorySlot_Accessory4,
		InventorySlot_Food1, InventorySlot_Food2, InventorySlot_Food3, InventorySlot_Food4, InventorySlot_Food5,
	};
	ensure(DC1EquipSlotLayout::SlotCount == EquipSlots.Num());

	for (int32 Index = 0; Index < EquipSlots.Num(); ++Index)
	{
		UDC1InventorySlotWidget* SlotWidget = EquipSlots[Index];
		if (false == ::IsValid(SlotWidget))
		{
			continue;
		}

		SlotWidget->InitSlot(Index, SlotSize, EDC1InventorySource::Equipment);
		SlotWidget->SetWeightVisibility(false);

		const TObjectPtr<UTexture2D>* EmptyIcon = EmptySlotIcons.Find(DC1EquipSlotLayout::GetSlotType(Index));
		SlotWidget->SetEmptyIcon(nullptr != EmptyIcon ? EmptyIcon->Get() : nullptr);
	}

	if (::IsValid(InventorySlot_SphereModule))
	{
		InventorySlot_SphereModule->InitSlot(INDEX_NONE, SlotSize, EDC1InventorySource::Equipment);
		InventorySlot_SphereModule->SetWeightVisibility(false);
		InventorySlot_SphereModule->ClearSlot();
		InventorySlot_SphereModule->SetLocked(true);
	}
}

void UDC1EquipmentWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (::IsValid(CachedViewModel))
	{
		// SetViewModel에서 이미 구독했을 수 있으므로 중복 구독 방지
		CachedViewModel->OnEquipmentUIUpdated.RemoveAll(this);
		CachedViewModel->OnEquipmentUIUpdated.AddUObject(this, &ThisClass::HandleEquipmentUpdated);
	}

	RefreshAllSlots();

	SetupPlayerPreview();
}

void UDC1EquipmentWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// 인벤토리 탭이 보이는 동안만 미리보기 촬영 - 다른 탭의 EndPreview 뒤에도 탭 복귀 시 다시 켜진다
	if (UDC1PreviewSubsystem* PreviewSubsystem = GetOwningLocalPlayer() ? GetOwningLocalPlayer()->GetSubsystem<UDC1PreviewSubsystem>() : nullptr)
	{
		PreviewSubsystem->BeginPlayerPreview();
	}
}

void UDC1EquipmentWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if (UDC1PreviewSubsystem* PreviewSubsystem = GetOwningLocalPlayer() ? GetOwningLocalPlayer()->GetSubsystem<UDC1PreviewSubsystem>() : nullptr)
	{
		PreviewSubsystem->EndPreview();
	}
}

void UDC1EquipmentWidget::NativeDestruct()
{
	if (::IsValid(CachedViewModel))
	{
		CachedViewModel->OnEquipmentUIUpdated.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UDC1EquipmentWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// 화면이 열려 있는 동안만 미리보기 촬영
	if (UDC1PreviewSubsystem* PreviewSubsystem = GetOwningLocalPlayer() ? GetOwningLocalPlayer()->GetSubsystem<UDC1PreviewSubsystem>() : nullptr)
	{
		PreviewSubsystem->BeginPlayerPreview();
	}
}

void UDC1EquipmentWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if (UDC1PreviewSubsystem* PreviewSubsystem = GetOwningLocalPlayer() ? GetOwningLocalPlayer()->GetSubsystem<UDC1PreviewSubsystem>() : nullptr)
	{
		PreviewSubsystem->EndPreview(EDC1PreviewType::Player);
	}
}

void UDC1EquipmentWidget::SetupPlayerPreview()
{
	if (::IsValid(PlayerPreviewMID) || false == ::IsValid(Image_CharacterPreview))
	{
		return;
	}

	const UDC1UIResourceData* ResourceData = UDC1AssetManager::GetAssetByTag<UDC1UIResourceData>(DC1AssetTags::Asset_UI_Resource_Common);
	UDC1PreviewSubsystem* PreviewSubsystem = GetOwningLocalPlayer() ? GetOwningLocalPlayer()->GetSubsystem<UDC1PreviewSubsystem>() : nullptr;
	if (nullptr == ResourceData || nullptr == ResourceData->PlayerPreviewMaterial || nullptr == PreviewSubsystem)
	{
		return;
	}

	PlayerPreviewMID = UMaterialInstanceDynamic::Create(ResourceData->PlayerPreviewMaterial, this);
	PlayerPreviewMID->SetTextureParameterValue(PlayerPreviewTextureParam, PreviewSubsystem->GetRenderTarget(EDC1PreviewType::Player));
	Image_CharacterPreview->SetBrushFromMaterial(PlayerPreviewMID);
}

void UDC1EquipmentWidget::SetViewModel(UDC1InventoryViewModel* InViewModel)
{
	if (::IsValid(CachedViewModel))
	{
		CachedViewModel->OnEquipmentUIUpdated.RemoveAll(this);
	}

	CachedViewModel = InViewModel;

	if (::IsValid(CachedViewModel))
	{
		CachedViewModel->OnEquipmentUIUpdated.AddUObject(this, &ThisClass::HandleEquipmentUpdated);
	}

	RefreshAllSlots();
}

int32 UDC1EquipmentWidget::GetEquipSlotIndexFromPosition(const FVector2D& InScreenPos) const
{
	for (int32 Index = 0; Index < EquipSlots.Num(); ++Index)
	{
		const UDC1InventorySlotWidget* SlotWidget = EquipSlots[Index];
		if (true == ::IsValid(SlotWidget) && true == SlotWidget->GetCachedGeometry().IsUnderLocation(InScreenPos))
		{
			return Index;
		}
	}

	return -1;
}

void UDC1EquipmentWidget::RefreshAllSlots()
{
	for (int32 Index = 0; Index < EquipSlots.Num(); ++Index)
	{
		UDC1InventorySlotWidget* SlotWidget = EquipSlots[Index];
		if (false == ::IsValid(SlotWidget))
		{
			continue;
		}

		// SetSlotData는 VM이 없거나 빈 칸이면 ClearSlot으로 처리
		SlotWidget->SetSlotData(CachedViewModel);

		// 빈 칸 아이콘 갱신 뒤에 적용해야 잠긴 칸에서 실루엣이 숨겨짐
		SlotWidget->SetLocked(::IsValid(CachedViewModel) && CachedViewModel->IsEquipSlotLocked(Index));
	}
}

void UDC1EquipmentWidget::HandleEquipmentUpdated()
{
	RefreshAllSlots();
}
