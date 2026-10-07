// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Equip/DC1EquipViewModel.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1EquipComponent.h"
#include "Component/Inventory/Player/DC1PlayerInventoryComponent.h"
#include "Item/Equip/Weapon/DC1Weapon.h"

void UDC1EquipViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	if (nullptr == InPlayerState)
		return;

	EquipComponent = InPlayerState->GetEquipComponent();
	if (nullptr == EquipComponent)
		return;

	EquipComponent->OnWeaponChanged.AddUObject(this, &ThisClass::HandleWeaponChanged);
}

void UDC1EquipViewModel::ClearSource()
{
	if(EquipComponent.IsValid())
		EquipComponent->OnWeaponChanged.RemoveAll(this);

	EquipComponent.Reset();
	InventoryComponent.Reset();
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
	// TODO : ADC1ProjectileWeapon에 장탄 프로퍼티가 생기면 캐스팅해서 읽는다
	return 0;
}

void UDC1EquipViewModel::HandleWeaponChanged()
{
	OnWeaponChanged.Broadcast();
}

void UDC1EquipViewModel::HandleAmmoChanged()
{
	OnAmmoChanged.Broadcast();
}
