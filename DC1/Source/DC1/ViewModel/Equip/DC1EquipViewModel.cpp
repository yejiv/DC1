// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Equip/DC1EquipViewModel.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1EquipComponent.h"
#include "Component/Inventory/Player/DC1PlayerInventoryComponent.h"
#include "Component/PalSphere/DC1PalSphereComponent.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1ItemHelper.h"
#include "Item/Equip/Weapon/DC1Weapon.h"
#include "Item/Equip/Weapon/RangedWeapon/DC1ProjectileWeapon.h"

void UDC1EquipViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	if (nullptr == InPlayerState)
		return;

	EquipComponent = InPlayerState->GetEquipComponent();
	if (nullptr == EquipComponent)
		return;

	EquipComponent->OnWeaponChanged.AddUObject(this, &ThisClass::HandleWeaponChanged);

	InventoryComponent = InPlayerState->GetInventoryComponent();
	if (InventoryComponent.IsValid())
		InventoryComponent->OnInventoryUpdated.AddUObject(this, &ThisClass::HandleInventoryChanged);

	PalSphereComponent = InPlayerState->GetPalSphereComponent();
	if (PalSphereComponent.IsValid())
		PalSphereComponent->OnSphereSelectionChanged.AddUObject(this, &ThisClass::HandleSphereSelectionChanged);
}

void UDC1EquipViewModel::ClearSource()
{
	if(EquipComponent.IsValid())
		EquipComponent->OnWeaponChanged.RemoveAll(this);

	if (InventoryComponent.IsValid())
		InventoryComponent->OnInventoryUpdated.RemoveAll(this);

	if (PalSphereComponent.IsValid())
		PalSphereComponent->OnSphereSelectionChanged.RemoveAll(this);

	EquipComponent.Reset();
	InventoryComponent.Reset();
	PalSphereComponent.Reset();
}

UTexture2D* UDC1EquipViewModel::GetCurrentWeaponIcon()
{
	if (false == EquipComponent.IsValid())
		return nullptr;

	ADC1Weapon* Weapon = EquipComponent->GetCurrentWeapon();

	return (nullptr != Weapon) ? Weapon->EquipIcon.LoadSynchronous() : nullptr;
}

bool UDC1EquipViewModel::UsesAmmo() const
{
	if (false == EquipComponent.IsValid())
		return false;

	const ADC1Weapon* Weapon = EquipComponent->GetCurrentWeapon();

	return (nullptr != Weapon) && Weapon->UsesAmmo();
}

int32 UDC1EquipViewModel::GetLoadedAmmo() const
{
	return (GetReserveAmmo() > 0) ? 1 : 0;
}

int32 UDC1EquipViewModel::GetReserveAmmo() const
{
	if (false == EquipComponent.IsValid() || false == InventoryComponent.IsValid())
		return 0;

	const ADC1ProjectileWeapon* Weapon = Cast<ADC1ProjectileWeapon>(EquipComponent->GetCurrentWeapon());
	if (nullptr == Weapon || true == Weapon->AmmoItemID.IsNone())
		return 0;

	return InventoryComponent->GetItemCount(Weapon->AmmoItemID);
}

int32 UDC1EquipViewModel::GetSelectedSphereCount() const
{
	if (false == PalSphereComponent.IsValid() || false == InventoryComponent.IsValid())
		return 0;

	const FName ItemID = PalSphereComponent->GetSelectedSphereItemID();
	return (true == ItemID.IsNone()) ? 0 : InventoryComponent->GetItemCount(ItemID);
}

UTexture2D* UDC1EquipViewModel::GetSelectedSphereIcon() const
{
	if (false == PalSphereComponent.IsValid())
		return nullptr;

	const FDC1ItemRow* ItemRow = DC1ItemHelper::FindItemRow(PalSphereComponent->GetSelectedSphereItemID());
	return (nullptr != ItemRow) ? ItemRow->ItemIcon.LoadSynchronous() : nullptr;
}

void UDC1EquipViewModel::HandleWeaponChanged()
{
	OnWeaponChanged.Broadcast();
}

void UDC1EquipViewModel::HandleAmmoChanged()
{
	OnAmmoChanged.Broadcast();
}

void UDC1EquipViewModel::HandleInventoryChanged()
{
	// 화살·스피어 보유 수가 같이 바뀐다
	OnAmmoChanged.Broadcast();
	OnSphereChanged.Broadcast();
}

void UDC1EquipViewModel::HandleSphereSelectionChanged()
{
	OnSphereChanged.Broadcast();
}
