// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/DC1HUDEquipWidget.h"

#include "Components/Image.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/LocalPlayer.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Equip/DC1EquipViewModel.h"
#include "UI/Counter/DC1DigitCounter.h"

void UDC1HUDEquipWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
}

void UDC1HUDEquipWidget::NativeDestruct()
{
	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1HUDEquipWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		EquipViewModel = VMSubsystem->GetViewModel<UDC1EquipViewModel>();
		if (false == EquipViewModel.IsValid())
			return;

		EquipViewModel->OnWeaponChanged.AddUObject(this, &ThisClass::HandleWeaponDisplayChanged);
		EquipViewModel->OnAmmoChanged.AddUObject(this, &ThisClass::HandleAmmoDisplayChanged);
		HandleWeaponDisplayChanged();
	}
}

void UDC1HUDEquipWidget::UnbindViewModel()
{
	if (EquipViewModel.IsValid())
	{
		EquipViewModel->OnWeaponChanged.RemoveAll(this);
		EquipViewModel->OnAmmoChanged.RemoveAll(this);
	}
	EquipViewModel.Reset();
}

void UDC1HUDEquipWidget::HandleWeaponDisplayChanged()
{
	if (false == EquipViewModel.IsValid())
		return;

	// Weapon UI Change
	UTexture2D* Icon = EquipViewModel->GetCurrentWeaponIcon();
	Image_Icon_Weapon->SetBrushFromTexture(nullptr != Icon ? Icon : Texture_Default.Get());

	bool UsesAmmo = EquipViewModel->UsesAmmo();
	WS_DigitWeapon->SetActiveWidgetIndex(UsesAmmo);

	HandleAmmoDisplayChanged();
}

void UDC1HUDEquipWidget::HandleAmmoDisplayChanged()
{
	if (false == EquipViewModel.IsValid())
		return;

	Digit_WeaponCurrent->SetValue(EquipViewModel->GetLoadedAmmo());
	// Temp
	Digit_WeaponRemain->SetValue(15);
}
